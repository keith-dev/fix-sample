#include "Application.h"

#include <spdlog/spdlog.h>

Application::Application(std::string_view configFile) :
	configFile_(configFile),
	router_(this),
	model_(&router_),
	settings_(configFile_),
	storeFactory_(settings_),
	logFactory_(settings_),
	acceptor_(std::unique_ptr<FIX::Acceptor>(
		new FIX::SocketAcceptor(*this, storeFactory_, settings_, logFactory_))) {
}

Application::~Application() {
	if (!stopped())
		stop();
}

void Application::onCreate(const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
}
void Application::onLogon(const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
	state_ = State::LoggedOn;
}
void Application::onLogout(const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
	state_ = State::LoggedOut;
}

void Application::toAdmin(FIX::Message& msg, const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
}
void Application::fromAdmin(const FIX::Message& msg, const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
	router_.crack(msg, sessionID);
}

void Application::toApp(FIX::Message&, const FIX::SessionID&) {
}
void Application::fromApp(const FIX::Message& msg, const FIX::SessionID& sessionID) {
	router_.crack(msg, sessionID);
}

void Application::onMessage(const FIX44::MarketDataRequest& msg, const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
}

void Application::onMessage(const FIX44::BusinessMessageReject& msg, const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
}
