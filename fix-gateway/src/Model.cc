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

void Model::Unsubscribe(const std::string& symbol, const std::string& sessionID) {
	subscriber_.Unsubscribe(symbol, sessionID);
}

void Model::Unsubscribe(const std::string& sessionID) {
	subscriber_.Unsubscribe(sessionID);
}

void Model::Unsubscribe() {
	subscriber_.Unsubscribe();
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

void Model::Publish(std::string symbol, Subscribers subscribers, Orderbook orderbook) {
	for (const auto& [mdReqID, sessionID] : subscribers) {
		if (auto mdSnapshotMsg = create<FIX44::MarketDataSnapshotFullRefresh>(mdReqID, symbol, orderbook)) {
			FIX::Session::sendToTarget(*mdSnapshotMsg, toSessionID(sessionID));
		}
	}
}

template <>
std::unique_ptr<FIX44::MarketDataSnapshotFullRefresh> Model::create<FIX44::MarketDataSnapshotFullRefresh>(
	const std::string& mdReqID, const std::string& symbol, const Orderbook& orderbook) {
	auto mdSnapshotMsg = std::make_unique<FIX44::MarketDataSnapshotFullRefresh>();

	mdSnapshotMsg->setField({FIX::FIELD::MDReqID, mdReqID}, true);
	mdSnapshotMsg->setField({FIX::FIELD::Symbol, symbol}, true);

	for (const auto& order : orderbook) {
		FIX44::MarketDataSnapshotFullRefresh::NoMDEntries noMDEntries;
		(order.type == BidOffer::Bid)
			? noMDEntries.set(FIX::MDEntryType(FIX::MDEntryType_BID))
			: noMDEntries.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
		noMDEntries.set(FIX::MDEntryPx(order.price));
		noMDEntries.set(FIX::MDEntrySize(order.size));
		noMDEntries.set(FIX::OrderID(order.orderID));
		mdSnapshotMsg->addGroup(noMDEntries);
	}

	return mdSnapshotMsg;
}
