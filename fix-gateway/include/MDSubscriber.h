#pragma once

#include "concurrentqueue.h"

#include <wise_enum/wise_enum.h>

#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

WISE_ENUM_CLASS(BidOffer, Bid, Offer);
struct PriceSnapshot {
	BidOffer type;
	double price;
	int size;
	std::string orderID;
};

class MDPublisher {
public:
	MDPublisher() {
	}
	~MDPublisher() {
		stop();
	}

	void start() {
		stop();

		stopped_ = false;
		worker_ = std::thread([&]() {
				while (!stopped_) {
					using namespace std::literals::chrono_literals;
					std::this_thread::sleep_for(100ms);

					std::lock_guard<std::mutex> lock(mtx_);
					for (const auto& [symbol, subscribers] : subscribed_) {
						// take a copy of subscribers/orderbook for publishing
						std::unique_ptr<Subscribers> dup_subscribers(new Subscribers);
						*dup_subscribers = subscribers;

						std::unique_ptr<Orderbook> dup_orderbook(new Orderbook);
						*dup_orderbook = orderbooks_.find(symbol)->second;

						UpdateOrderbooks();
					}
				}
			});
	}

	void stop() {
		stopped_ = true;
		if (worker_.joinable()) {
			worker_.join();
		}
	}

	void UpdateOrderbooks() {
	}

	bool Subscribe(const std::string& symbol, const std::string& sessionID) {
		std::lock_guard<std::mutex> lock(mtx_);

		auto iter = orderbooks_.find(symbol);
		if (iter == orderbooks_.end())
			return false;

		subscribed_[symbol].insert(sessionID);
		return true;
	}

	bool Unsubscribe(const std::string& symbol, const std::string& sessionID) {
		std::lock_guard<std::mutex> lock(mtx_);

		auto iter = subscribed_.find(symbol);
		if (iter == subscribed_.end())
			return false;

		auto subscriber_iter = iter->second.find(sessionID);
		if (subscriber_iter == iter->second.end())
			return false;

		iter->second.erase(subscriber_iter);
		return true;
	}

private:
	using Orderbook   = std::vector<PriceSnapshot>;
	using Orderbooks  = std::unordered_map<std::string, Orderbook>;
	using Subscriber  = std::string; // std::string.toStringFrozen()
	using Subscribers = std::unordered_set<Subscriber>;
	using Subscribed  = std::unordered_map<std::string, Subscribers>;

	Orderbooks orderbooks_;
	Subscribed subscribed_;

	std::atomic<bool> stopped_;
	std::mutex mtx_;
	std::thread worker_;
};
