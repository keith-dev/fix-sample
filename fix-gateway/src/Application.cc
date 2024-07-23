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
	if (!stopped()) {
		model_.Unsubscribe();
		stop();
	}
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
	model_.Unsubscribe(sessionID.toStringFrozen());
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

	FIX::MDReqID mdReqID;
	msg.get(mdReqID);
	FIX::SubscriptionRequestType subscriptionReqType;
	msg.get(subscriptionReqType);
	for (const auto& symbol : model_.getSymbols(msg)) {
		switch (subscriptionReqType) {
		case FIX::SubscriptionRequestType_SNAPSHOT:
		case FIX::SubscriptionRequestType_SNAPSHOT_AND_UPDATES:
			model_.Subscribe(mdReqID, symbol, sessionID.toStringFrozen());
			break;
		case FIX::SubscriptionRequestType_DISABLE_PREVIOUS_SNAPSHOT:
			model_.Unsubscribe(symbol, sessionID.toStringFrozen());
			break;
		}
	}
}

void Application::onMessage(const FIX44::BusinessMessageReject& msg, const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
}
