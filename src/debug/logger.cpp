#include <mars/debug/logger.hpp>

#include <mars/debug/crash_handler.hpp>
#include <mars/debug/env.hpp>
#include <mars/parser/json/json.hpp>

#include <cstdio>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace mars::logger {
namespace {

struct log_entry_json {
	std::string level;
	std::string channel;
	std::string message;
};

struct crash_entry_json {
	std::string message;
	std::vector<std::string> stack;
};


static void logger_crash_handler(const mars::debug::crash_data& _data);
static void finalize_json_log();

struct logger_sink_state {
	std::mutex mutex;
	std::ofstream file;
	bool file_enabled = false;
	std::ofstream json_file;
	bool json_first_entry = true;
	int m_sublog_depth = 0;
	std::vector<bool> m_sublog_first_content_stack;

	logger_sink_state() {
		if constexpr (!mars::env::formatted_output) {
			file.open("log.txt", std::ios::out | std::ios::trunc);
			file_enabled = file.is_open();
		}
		else {
			json_file.open("log.json", std::ios::out | std::ios::trunc);
			json_file << "{\"log\":[\n";
			json_file.flush();
			mars::debug::set_crash_handler(&logger_crash_handler);
			std::atexit([] { finalize_json_log(); });
		}
	}

	~logger_sink_state() {
		if constexpr (mars::env::formatted_output) {
			if (json_file.is_open()) {
				for (int i = 0; i < m_sublog_depth; ++i)
					json_file << "\n]}";
				json_file << "\n]}\n";
				json_file.flush();
			}
		}
	}
};

logger_sink_state& sink_state() {
	static logger_sink_state state;
	return state;
}

logger_sink_state& startup_sink_state = sink_state();

static void finalize_json_log() {
	auto& s = sink_state();
	std::lock_guard lock(s.mutex);
	if (!s.json_file.is_open())
		return;
	for (int i = 0; i < s.m_sublog_depth; ++i)
		s.json_file << "\n]}";
	s.m_sublog_depth = 0;
	s.m_sublog_first_content_stack.clear();
	s.json_file << "\n]}\n";
	s.json_file.flush();
	s.json_file.close();
}

static void finalize_json_log_with_crash(logger_sink_state& _s, const mars::debug::crash_data& _data) {
	if (!_s.json_file.is_open())
		return;

	for (int i = 0; i < _s.m_sublog_depth; ++i)
		_s.json_file << "\n]}";
	_s.m_sublog_depth = 0;
	_s.m_sublog_first_content_stack.clear();

	std::vector<std::string> frames;
	std::istringstream ss(_data.callstack);
	std::string line;
	while (std::getline(ss, line))
		if (!line.empty())
			frames.push_back(std::move(line));

	crash_entry_json ce{ _data.reason, std::move(frames) };
	std::string crash_json;
	mars::json::json_type_parser<crash_entry_json>::stringify(ce, crash_json);

	_s.json_file << "\n],\"crash\":" << crash_json << "}\n";
	_s.json_file.flush();
	_s.json_file.close();
}

static void logger_crash_handler(const mars::debug::crash_data& _data) {
	finalize_json_log_with_crash(sink_state(), _data);
}

static void write_sublog_begin(const std::string& _channel_name, const std::string& _name) {
	auto& s = sink_state();
	std::lock_guard lock(s.mutex);

	if constexpr (!mars::env::formatted_output) {
		const std::string line = std::format("[sublog: {}/{}] {{", _channel_name, _name);
		std::fputs(line.c_str(), stdout);
		std::fputc('\n', stdout);
		std::fflush(stdout);
		if (s.file_enabled) {
			s.file << line << '\n';
			s.file.flush();
		}
	} else {
		std::fputs(std::format("[sublog] | {} | {}\n", _channel_name, _name).c_str(), stderr);
		std::fflush(stderr);

		std::string name_copy(_name);
		std::string name_json;
		mars::json::json_type_parser<std::string>::stringify(name_copy, name_json);
		const std::string header = std::format("{{\"level\":\"sublog\",\"name\":{},\"content\":[", name_json);

		std::fputs(header.c_str(), stdout);
		std::fflush(stdout);

		if (s.json_file.is_open()) {
			if (s.m_sublog_depth == 0) {
				if (!s.json_first_entry)
					s.json_file << ",\n";
				s.json_first_entry = false;
			} else {
				if (!s.m_sublog_first_content_stack.back())
					s.json_file << ",\n";
				s.m_sublog_first_content_stack.back() = false;
			}
			s.json_file << header;
			s.json_file.flush();
			s.m_sublog_first_content_stack.push_back(true);
		}
		++s.m_sublog_depth;
	}
}

static void write_sublog_end() {
	auto& s = sink_state();
	std::lock_guard lock(s.mutex);

	if constexpr (!mars::env::formatted_output) {
		std::fputs("}\n", stdout);
		std::fflush(stdout);
		if (s.file_enabled) {
			s.file << "}\n";
			s.file.flush();
		}
	} else {
		if (s.m_sublog_depth > 0) {
			--s.m_sublog_depth;
			if (!s.m_sublog_first_content_stack.empty())
				s.m_sublog_first_content_stack.pop_back();
			std::fputs("\n]}", stdout);
			std::fflush(stdout);
			if (s.json_file.is_open()) {
				s.json_file << "\n]}";
				s.json_file.flush();
			}
		}
	}
}

} // namespace

void write_formatted_line(const std::string& _line) {
	auto& s = sink_state();
	std::lock_guard lock(s.mutex);

	std::fputs(_line.c_str(), stdout);
	std::fputc('\n', stdout);
	std::fflush(stdout);

	if (!s.file_enabled)
		return;

	s.file << _line << '\n';
	s.file.flush();
	if (!s.file) {
		s.file_enabled = false;
		s.file.close();
	}
}

void write_json_entry(const std::string& _level, const std::string& _channel, const std::string& _message) {
	auto& s = sink_state();
	std::lock_guard lock(s.mutex);

	std::fputs(std::format("[{}] | {} | {}\n", _level, _channel, _message).c_str(), stderr);
	std::fflush(stderr);

	log_entry_json entry{ _level, _channel, _message };
	std::string json_obj;
	mars::json::json_type_parser<log_entry_json>::stringify(entry, json_obj);

	std::fputs(json_obj.c_str(), stdout);
	std::fputc('\n', stdout);
	std::fflush(stdout);

	if (s.json_file.is_open()) {
		if (s.m_sublog_depth > 0) {
			if (!s.m_sublog_first_content_stack.back())
				s.json_file << ",\n";
			s.m_sublog_first_content_stack.back() = false;
		} else {
			if (!s.json_first_entry)
				s.json_file << ",\n";
			s.json_first_entry = false;
		}
		s.json_file << json_obj;
		s.json_file.flush();
	}
}

scoped_log::scoped_log(scoped_log&& _other) noexcept {
	m_active = _other.m_active;
	_other.m_active = false;
}

scoped_log::~scoped_log() {
	if (m_active)
		write_sublog_end();
}

scoped_log begin_sublog(const log_channel& _channel, std::string_view _name) {
	write_sublog_begin(_channel.name(), std::string(_name));
	scoped_log result;
	result.m_active = true;
	return result;
}

} // namespace mars::logger
