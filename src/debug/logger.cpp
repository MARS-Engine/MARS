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
	s.json_file << "\n]}\n";
	s.json_file.flush();
	s.json_file.close();
}

static void finalize_json_log_with_crash(logger_sink_state& _s, const mars::debug::crash_data& _data) {
	if (!_s.json_file.is_open())
		return;

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
		if (!s.json_first_entry)
			s.json_file << ",\n";
		s.json_first_entry = false;
		s.json_file << json_obj;
		s.json_file.flush();
	}
}

} // namespace mars::logger
