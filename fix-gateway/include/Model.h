#pragma once

#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
//#include <quickfix/fix50sp2/MarketDataRequest.h>

#include <wise_enum/wise_enum.h>

#include <memory>
#include <string>
#include <vector>

namespace FIX44 {
class MarketDataRequest;
}

class Router;

class Model {
	Router* router_;

public:
	Model(Router* router) : router_(router) {
	}

	std::vector<std::string> getSymbols(const FIX44::MarketDataRequest& msg);

	template <typename U>
	std::unique_ptr<U> create(const std::vector<std::string>& symbols) {
		return {};
	}

	template <typename U>
	std::unique_ptr<U> create(const std::string& symbol) {
		return {};
	}
};

WISE_ENUM_CLASS(BidOffer, Bid, Offer);
struct PriceSnapshot {
	BidOffer type;
	double price;
	double size;
	std::string orderId;
};

template <>
inline
std::unique_ptr<FIX44::MarketDataSnapshotFullRefresh> Model::create<FIX44::MarketDataSnapshotFullRefresh>(const std::string& symbol) {
	auto mdSnapshotMsg = std::make_unique<FIX44::MarketDataSnapshotFullRefresh>();
	mdSnapshotMsg->setField({FIX::FIELD::Symbol, symbol}, true);

	static std::array<PriceSnapshot, 6> orders {
		{BiddOffer::Bid, 0.99, 3, "test::order::1"}, {BiddOffer::Offer, 1.01, 3, "test::order::1"},
		{BiddOffer::Bid, 0.99, 3, "test::order::2"}, {BiddOffer::Offer, 1.01, 3, "test::order::2"},
		{BiddOffer::Bid, 0.99, 3, "test::order::3"}, {BiddOffer::Offer, 1.01, 3, "test::order::3"}
	};
	FIX44::MarketDataRequest::NoMDEntryTypes noMDEntryTypes;
	for (const auto& entry : orders) {
		noMDEntryTypes.set(entry);
		mdSnapshotMsg->
	}

/*
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
 */
	return mdSnapshotMsg;
}
