#pragma once

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

#include <spdlog/spdlog.h>
#include <wise_enum/wise_enum.h>

#include <atomic>
#include <memory>
#include <string>
#include <string_view>

class Application : public FIX::Application {
public:
	Application(std::string_view configFile) :
		configFile_(configFile),
		router_(this),
		model_(&router_),
		settings_(configFile_),
		storeFactory_(settings_),
		logFactory_(settings_),
		initiator_(std::unique_ptr<FIX::Initiator>(
			new FIX::SocketInitiator(*this, storeFactory_, settings_, logFactory_))) {
	}
	~Application() override {
		if (!stopped())
			stop();
	}

	void start() { initiator_->start(); }
	void stop()  { initiator_->stop(); stopped_ = true; }
	bool stopped() const  { return stopped_.load(); }
	bool loggedOn() const { return state_ == State::LoggedOn; }

	void onCreate(const FIX::SessionID& sessionID) override {
		spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
	}
	void onLogon(const FIX::SessionID& sessionID) override {
		spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
		state_ = State::LoggedOn;
	}
	void onLogout(const FIX::SessionID& sessionID) override {
		spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
		state_ = State::LoggedOut;
	}

	void toAdmin(FIX::Message& msg, const FIX::SessionID& sessionID) override {
		spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
		FIX::BoolField resetSeqNum(FIX::FIELD::ResetSeqNumFlag, true);
//		msg.setField(resetSeqNum, true); // with overwrite
	}
	void fromAdmin(const FIX::Message& msg, const FIX::SessionID& sessionID) override {
		spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
		router_.crack(msg, sessionID);
	}

	void toApp(FIX::Message&, const FIX::SessionID&) override {
	}
	void fromApp(const FIX::Message& msg, const FIX::SessionID& sessionID) override {
		router_.crack(msg, sessionID);
	}

	void subscribe(const std::vector<std::string>& symbols, const FIX::SessionID& sessionID) {
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
	void onMessage(const FIX44::MarketDataSnapshotFullRefresh& msg, const FIX::SessionID& sessionID) {
		spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
	}
	void onMessage(const FIX44::BusinessMessageReject& msg, const FIX::SessionID& sessionID) {
		spdlog::info("{} sessionID={}", __PRETTY_FUNCTION__, sessionID.toString());
	}

private:
	WISE_ENUM_CLASS_MEMBER(State, LoggedOut, LoggedOn);

	std::atomic<State> state_{State::LoggedOut};
	std::atomic<bool> stopped_{true};

	std::string configFile_;
	Router router_;
	Model model_;
	FIX::SessionSettings settings_;
	FIX::FileStoreFactory storeFactory_;
	FIX::ScreenLogFactory logFactory_;
	std::unique_ptr<FIX::Initiator> initiator_;
};
