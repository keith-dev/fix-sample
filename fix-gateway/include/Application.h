#pragma once

#include "Model.h"
#include "Router.h"

#include <quickfix/Application.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/FileStore.h>
#include <quickfix/Log.h>
#include <quickfix/SocketAcceptor.h>
/*
#include <quickfix/Session.h>

#include <quickfix/fix44/BusinessMessageReject.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include <spdlog/spdlog.h>
 */
#include <wise_enum/wise_enum.h>

#include <atomic>
#include <memory>
#include <string>
#include <string_view>

namespace FIX44 {
class MarketDataRequest;
class BusinessMessageReject;
}

class Application : public FIX::Application {
public:
	Application(std::string_view configFile);
	~Application() override;

	void start() { acceptor_->start(); model_.start(); }
	void stop()  { acceptor_->stop(); model_.stop(); stopped_ = true; }
	bool stopped() const  { return stopped_.load(); }
	bool loggedOn() const { return state_ == State::LoggedOn; }

	void onCreate(const FIX::SessionID& sessionID) override;
	void onLogon(const FIX::SessionID& sessionID) override;
	void onLogout(const FIX::SessionID& sessionID) override;

	void toAdmin(FIX::Message& msg, const FIX::SessionID& sessionID) override;
	void fromAdmin(const FIX::Message& msg, const FIX::SessionID& sessionID) override;

	void toApp(FIX::Message&, const FIX::SessionID&) override;
	void fromApp(const FIX::Message& msg, const FIX::SessionID& sessionID) override;

	void onMessage(const FIX44::MarketDataRequest& msg, const FIX::SessionID& sessionID);
	void onMessage(const FIX44::BusinessMessageReject& msg, const FIX::SessionID& sessionID);

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
	std::unique_ptr<FIX::Acceptor> acceptor_;
};
