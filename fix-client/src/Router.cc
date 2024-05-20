#include "Router.h"
#include "Application.h"

void Router::onMessage(const FIX44::MarketDataSnapshotFullRefresh& msg, const FIX::SessionID& sessionID) {
	application_->onMessage(msg, sessionID);
}

void Router::onMessage(const FIX44::BusinessMessageReject& msg, const FIX::SessionID& sessionID) {
	application_->onMessage(msg, sessionID);
}
