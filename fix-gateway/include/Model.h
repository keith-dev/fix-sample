#pragma once

#include "MDPublisher.h"

#include <quickfix/Session.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include <wise_enum/wise_enum.h>

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Router;

class Model {
	using Subscribers = MDPublisher::Subscribers;
	using Orderbook   = MDPublisher::Orderbook;

	Router* router_;
	MDPublisher subscriber_;

public:
	Model(Router* router) : router_(router), subscriber_(Publish) {
	}

	void Subscribe(const std::string& mdReqID, const std::string& symbol, const std::string& sessionID);
	static void Publish(std::string symbol, Subscribers subscribers, Orderbook orderbook);

	std::vector<std::string> getSymbols(const FIX44::MarketDataRequest& msg);

	template <typename U>
	static std::unique_ptr<U> create(const std::vector<std::string>& symbols) {
		return {};
	}

	template <typename U>
	static std::unique_ptr<U> create(const std::string& mdReqID, const std::string& symbol) {
		return {};
	}
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

	return mdSnapshotMsg;
}
