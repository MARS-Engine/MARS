#include <mars/debug/logger.hpp>

#include <cstdio>
#include <fstream>
#include <mutex>
#include <string>

namespace mars::logger {
namespace {

struct logger_sink_state {
	std::mutex mutex;
	std::ofstream file;
	bool file_enabled = false;

	logger_sink_state() {
		file.open("log.txt", std::ios::out | std::ios::trunc);
		file_enabled = file.is_open();
	}
};

logger_sink_state& sink_state() {
	static logger_sink_state state;
	return state;
}

logger_sink_state& startup_sink_state = sink_state();

} // namespace

void write_formatted_line(const std::string& line) {
	auto& state = sink_state();
	std::lock_guard lock(state.mutex);

	std::fputs(line.c_str(), stdout);
	std::fputc('\n', stdout);
	std::fflush(stdout);

	if (!state.file_enabled)
		return;

	state.file << line << '\n';
	state.file.flush();
	if (!state.file) {
		state.file_enabled = false;
		state.file.close();
	}
}

} // namespace mars::logger
