#include "Application.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <csignal>
#include <cstring>
#include <memory>
#include <string_view>

namespace {
std::atomic<bool> stop{false};
std::unique_ptr<Application> app;

void stop_handler(int) {
	stop = true;
}
}  // namespace

int main(int argc, char* argv[]) {
	std::string_view configFile{"fix-gateway.cfg"};
	if (argc > 1)
		configFile = std::string_view{argv[1], std::strlen(argv[1])};

	std::signal(SIGINT, stop_handler);

	try {
		using namespace std::literals::chrono_literals;

		app = std::make_unique<Application>(configFile);
		app->start();
		while (!stop)
			std::this_thread::sleep_for(1s);
		app->stop();
		spdlog::info("{}", "stopping");
	}
	catch (const std::exception& e) {
		spdlog::critical("{}", e.what());
	}
}
