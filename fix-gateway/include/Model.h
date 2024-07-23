#pragma once

#include "MDPublisher.h"

#include <quickfix/Session.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include <wise_enum/wise_enum.h>

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

	void start() {
		subscriber_.start();
	}

	void stop() {
		subscriber_.stop();
	}

	void Subscribe(const std::string& mdReqID, const std::string& symbol, const std::string& sessionID);
	std::vector<std::string> getSymbols(const FIX44::MarketDataRequest& msg);

	static void Publish(std::string symbol, Subscribers subscribers, Orderbook orderbook);

	static FIX::SessionID toSessionID(const std::string& str);

	template <typename U>
	static std::unique_ptr<U> create(const std::vector<std::string>& symbols) {
		return {};
	}

	template <typename U>
	static std::unique_ptr<U> create(const std::string& mdReqID, const std::string& symbol, const Orderbook& orderbook) {
		return {};
	}
};

template <>
std::unique_ptr<FIX44::MarketDataSnapshotFullRefresh> Model::create<FIX44::MarketDataSnapshotFullRefresh>(
	const std::string& mdReqID, const std::string& symbol, const Orderbook& orderbook);
