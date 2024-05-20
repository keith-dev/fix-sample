#pragma once

#include <quickfix/MessageCracker.h>

class Application;

// Route recognised received messages back to the application
class Router : public FIX::MessageCracker {
public:
	explicit Router(Application* application) : application_(application) {
	}

	Application* application() { return application_; }

	void onMessage(const FIX44::MarketDataRequest& msg, const FIX::SessionID& sessionID);
	void onMessage(const FIX44::BusinessMessageReject& msg, const FIX::SessionID& sessionID);

private:
	Application* application_;
};
