#include "Application.h"

/*
#include "Model.h"
#include "Router.h"

#include <quickfix/Application.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/FileStore.h>
#include <quickfix/Log.h>
#include <quickfix/Session.h>
#include "quickfix/SocketInitiator.h"

#include <quickfix/fix44/BusinessMessageReject.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
 */

#include <spdlog/spdlog.h>
//#include <wise_enum/wise_enum.h>

//#include <atomic>
//#include <memory>
//#include <string>
//#include <string_view>

Application::Application(std::string_view configFile) :
	configFile_(configFile),
	router_(this),
	model_(&router_),
	settings_(configFile_),
	storeFactory_(settings_),
	logFactory_(settings_),
	initiator_(std::unique_ptr<FIX::Initiator>(
		new FIX::SocketInitiator(*this, storeFactory_, settings_, logFactory_))) {
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
	FIX::BoolField resetSeqNum(FIX::FIELD::ResetSeqNumFlag, true);
//	msg.setField(resetSeqNum, true); // with overwrite
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

void Application::subscribe(const std::vector<std::string>& symbols, const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
	if (sessionID.getTargetCompID() == "MD_GW6") {
		if (auto mdReq = model_.create<FIX44::MarketDataRequest>(symbols))
			FIX::Session::sendToTarget(*mdReq, sessionID);
	}
	if (sessionID.getTargetCompID() == "MD_GW9") {
		if (auto mdReq = model_.create<FIX50SP2::MarketDataRequest>({"GBP/USD", "EUR/JPY"}))
			FIX::Session::sendToTarget(*mdReq, sessionID);
	}
}
void Application::onMessage(const FIX44::MarketDataSnapshotFullRefresh& msg, const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
}
void Application::onMessage(const FIX44::BusinessMessageReject& msg, const FIX::SessionID& sessionID) {
	spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
}
