#pragma once

#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix50sp2/MarketDataRequest.h>

#include <array>
#include <memory>
#include <string>
#include <vector>

class Router;

class Model {
	Router* router_;
	int mdReqCount{};

public:
	Model(Router* router) : router_(router) {
	}

	template <typename U>
	std::unique_ptr<U> create(const std::vector<std::string>& symbols) {
		return {};
	}
};

template <>
inline
std::unique_ptr<FIX44::MarketDataRequest> Model::create<FIX44::MarketDataRequest>(const std::vector<std::string>& symbols) {
	auto mdReqMsg = std::make_unique<FIX44::MarketDataRequest>(
		FIX::MDReqID("req:" + std::to_string(++mdReqCount)),
		FIX::SubscriptionRequestType(FIX::SubscriptionRequestType_SNAPSHOT),
		FIX::MarketDepth(0));

	FIX44::MarketDataRequest::NoMDEntryTypes noMDEntryTypes;
	for (const auto& entry : std::array<FIX::MDEntryType, 2>{FIX::MDEntryType_BID, FIX::MDEntryType_OFFER}) {
		noMDEntryTypes.set(entry);
		mdReqMsg->addGroup(noMDEntryTypes);
	}

	FIX44::MarketDataRequest::NoRelatedSym noRelatedSym;
	for (const auto& symbol : symbols) {
		noRelatedSym.setField(FIX::Symbol(symbol));
		mdReqMsg->addGroup(noRelatedSym);
	}

	return mdReqMsg;
}

template <>
inline
std::unique_ptr<FIX50SP2::MarketDataRequest> Model::create<FIX50SP2::MarketDataRequest>(const std::vector<std::string>& symbols) {
	auto mdReqMsg = std::make_unique<FIX50SP2::MarketDataRequest>(
		FIX::MDReqID("req:" + std::to_string(++mdReqCount)),
		FIX::SubscriptionRequestType(FIX::SubscriptionRequestType_SNAPSHOT),
		FIX::MarketDepth(0));

	FIX50SP2::MarketDataRequest::NoMDEntryTypes noMDEntryTypes;
	noMDEntryTypes.set(FIX::MDEntryType(FIX::MDEntryType_BID));
	mdReqMsg->addGroup(noMDEntryTypes);

	FIX50SP2::MarketDataRequest::NoRelatedSym noRelatedSym;
	noRelatedSym.setField(FIX::Symbol(symbols.front()));
	mdReqMsg->addGroup(noRelatedSym);

	return mdReqMsg;
}
