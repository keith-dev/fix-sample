#include "Model.h"

#include <spdlog/spdlog.h>

#include <quickfix/fix44/MarketDataRequest.h>

#include <cstring>

FIX::SessionID Model::toSessionID(const std::string& str) {
	const char* s = str.c_str();
	const char* p = std::strchr(s, ':');
	if (!p)
		return {};
	std::string beginString{s, static_cast<size_t>(p - s)};

	const char* q = std::strstr(s, "->");
	if (!q)
		return {};
	std::string senderCompID(p + 1, static_cast<size_t>(q - p - 1)); // offset :
	std::string targetCompID(q + 2); // offset ->

	spdlog::warn("\"{}:{}->{}\"", beginString, senderCompID, targetCompID);
	return FIX::SessionID(beginString, senderCompID, targetCompID);
}

void Model::Subscribe(const std::string& mdReqID, const std::string& symbol, const std::string& sessionID) {
	subscriber_.Subscribe(mdReqID, symbol, sessionID);
}

void Model::Publish(std::string symbol, Subscribers subscribers, Orderbook orderbook) {
	for (const auto& [mdReqID, sessionID] : subscribers) {
		if (auto mdSnapshotMsg = create<FIX44::MarketDataSnapshotFullRefresh>(mdReqID, symbol, orderbook)) {
			FIX::Session::sendToTarget(*mdSnapshotMsg, toSessionID(sessionID));
		}
	}
}

std::vector<std::string> Model::getSymbols(const FIX44::MarketDataRequest& msg) {
	std::vector<std::string> symbols;

	FIX::NoRelatedSym noRelatedSym;
	msg.getField(noRelatedSym);

	for (int i = 1; i <= noRelatedSym; ++i) {
		FIX44::MarketDataRequest::NoRelatedSym noRelatedSymGroup;
		msg.getGroup(1, noRelatedSymGroup);

		FIX::Symbol symbol;
		noRelatedSymGroup.get(symbol);
		symbols.push_back(symbol.getValue());
	}

	return symbols;
}

