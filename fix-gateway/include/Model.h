#pragma once

#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
//#include <quickfix/fix50sp2/MarketDataRequest.h>

#include <wise_enum/wise_enum.h>

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

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
	std::unique_ptr<U> create(const std::string& mdReqID, const std::string& symbol) {
		return {};
	}
};

WISE_ENUM_CLASS(BidOffer, Bid, Offer);
struct PriceSnapshot {
	BidOffer type;
	double price;
	int size;
	std::string orderID;
};

template <>
inline
std::unique_ptr<FIX44::MarketDataSnapshotFullRefresh> Model::create<FIX44::MarketDataSnapshotFullRefresh>(
	const std::string& mdReqID, const std::string& symbol) {
	auto mdSnapshotMsg = std::make_unique<FIX44::MarketDataSnapshotFullRefresh>();
	mdSnapshotMsg->setField({FIX::FIELD::MDReqID, mdReqID}, true);
	mdSnapshotMsg->setField({FIX::FIELD::Symbol, symbol}, true);

	static std::array<PriceSnapshot, 2> orders {
		PriceSnapshot{BidOffer::Bid, 0.99, 3, "test::order::3"},
		PriceSnapshot{BidOffer::Offer, 1.01, 3, "test::order::3"}
	};
	for (const auto& order : orders) {
		FIX44::MarketDataSnapshotFullRefresh::NoMDEntries noMDEntries;
		(order.type == BidOffer::Bid)
			? noMDEntries.set(FIX::MDEntryType(FIX::MDEntryType_BID))
			: noMDEntries.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
		noMDEntries.set(FIX::MDEntryPx(order.price));
		noMDEntries.set(FIX::MDEntrySize(order.size));
		noMDEntries.set(FIX::OrderID(order.orderID));
		mdSnapshotMsg->addGroup(noMDEntries);
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
