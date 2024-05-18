#pragma once

#include <quickfix/Application.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/FileStore.h>
#include <quickfix/Log.h>
#include <quickfix/SocketAcceptor.h>

#include <atomic>
#include <memory>
#include <string>
#include <string_view>

class Application : public FIX::Application {
public:
	Application(std::string_view configFile) :
		configFile_(configFile),
		settings_(configFile_),
		storeFactory_(settings_),
		logFactory_(settings_),
		acceptor_(std::unique_ptr<FIX::Acceptor>(
			new FIX::SocketAcceptor(*this, storeFactory_, settings_, logFactory_))) {
	}
	~Application() override {
		if (!stopped())
			stop();
	}

	void start() { acceptor_->start(); }
	void stop()  { acceptor_->stop(); stopped_ = true; }
	bool stopped() const { return stopped_.load(); }

	void onCreate(const FIX::SessionID&) override {}
	void onLogon(const FIX::SessionID&) override {}
	void onLogout(const FIX::SessionID&) override {}

	void toAdmin(FIX::Message&, const FIX::SessionID&) override {}
	void fromAdmin(const FIX::Message&, const FIX::SessionID&) override {}
	void toApp(FIX::Message&, const FIX::SessionID&) override {}
	void fromApp(const FIX::Message&, const FIX::SessionID&) override {}

private:
	std::atomic<bool> stopped_{true};

	std::string configFile_;
	FIX::SessionSettings settings_;
	FIX::FileStoreFactory storeFactory_;
	FIX::ScreenLogFactory logFactory_;
	std::unique_ptr<FIX::Acceptor> acceptor_;
};
