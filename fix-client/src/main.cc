#include "Application.h"

#include <quickfix/Session.h>

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
		while (!stop && !app->loggedOn())
			std::this_thread::sleep_for(1s);

		spdlog::info("{} sessions registered", FIX::Session::getSessions().size());
		const auto sessionID = FIX::SessionID("FIX.4.4", "CLIENT1", "MD_GW6");
		app->subscribe({"GBP/USD", "EUR/JPY"}, sessionID);
		std::this_thread::sleep_for(2s);

		app->stop();
		spdlog::info("{}", "stopping");
	}
	catch (const std::exception& e) {
		spdlog::critical("{}", e.what());
	}
}
