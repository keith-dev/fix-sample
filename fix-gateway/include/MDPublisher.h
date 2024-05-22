#pragma once

#include "concurrentqueue.h"

#include <wise_enum/wise_enum.h>

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
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

// A hash function used to hash a pair of any kind
// https://www.geeksforgeeks.org/how-to-create-an-unordered_map-of-pairs-in-c/
struct hash_pair {
	template <class T1, class T2>
	size_t operator()(const std::pair<T1, T2>& p) const {
		auto hash1 = std::hash<T1>{}(p.first);
		auto hash2 = std::hash<T2>{}(p.second);
 
		// If hash1 == hash2, their XOR is zero.
		if (hash1 == hash2) {
			return hash1;
		}
		return hash1 ^ hash2;			  
	}
};

class MDPublisher {
public:
	using Orderbook   = std::vector<PriceSnapshot>;
	using Orderbooks  = std::unordered_map<std::string, Orderbook>;
	using Subscriber  = std::pair<std::string, std::string>; // FIX::MDReqID, FIX::SessionID.toStringFrozen()
	using Subscribers = std::unordered_set<Subscriber, hash_pair>;
	using Subscribed  = std::unordered_map<std::string, Subscribers>; // symbol->mdreq/sessionIDs
	using Callback	  = std::function<void(std::string, Subscribers, Orderbook)>;

	explicit MDPublisher(Callback publish = nullptr) : publish_(publish) {
	}
	~MDPublisher() {
		stop();
	}

	void start() {
		stop();

		stopped_ = false;
		update_ = std::thread([&]() {
				while (!stopped_) {
					using namespace std::literals::chrono_literals;
					std::this_thread::sleep_for(500ms);

					std::lock_guard<std::mutex> lock(mtx_);
					for (const auto& [symbol, subscribers] : subscribed_) {
						// take a copy of subscribers/orderbook for publishing
						std::shared_ptr<QData> qdata(new QData{symbol, subscribers, orderbooks_.find(symbol)->second});
						queue_.enqueue(qdata);

						UpdateOrderbooks();
					}
				}
			});
		worker_ = std::thread([&]() {
				std::shared_ptr<QData> qdata;
				while (!stopped_) {
					while (queue_.try_dequeue(qdata)) {
						if (publish_) {
							publish_(std::move(qdata->symbol), std::move(qdata->subscribers), std::move(qdata->orderbook));
						}
						qdata.reset();
					}

					using namespace std::literals::chrono_literals;
					std::this_thread::sleep_for(50ms);
				}
			});
	}

	void stop() {
		stopped_ = true;
		if (update_.joinable()) {
			update_.join();
		}
		if (worker_.joinable()) {
			worker_.join();
		}
	}

	void UpdateOrderbooks() {
	}

	bool Subscribe(const std::string& mdReqID, const std::string& symbol, const std::string& sessionID) {
		std::lock_guard<std::mutex> lock(mtx_);

		// symbol valid?
		auto iter = orderbooks_.find(symbol);
		if (iter == orderbooks_.end())
			return false;

		// sessionID subscribed?
		auto subscribed_iter = subscribed_.find(symbol);
		if (subscribed_iter != subscribed_.end()) {
			for (const auto& [mdReqIDLocal, sessionIDLocal] : subscribed_iter->second) {
				if (sessionIDLocal == sessionID)
					return false;
			}
		}

		subscribed_[symbol].insert(std::make_pair(mdReqID, sessionID));
		return true;
	}

	bool Unsubscribe(const std::string& symbol, const std::string& sessionID) {
		std::lock_guard<std::mutex> lock(mtx_);

		auto iter = subscribed_.find(symbol);
		if (iter == subscribed_.end())
			return false;

		// erase all mdReqIDs matching subscribeID
		int count{};
		while (true) {
			auto subscriber_iter = std::find_if(iter->second.begin(), iter->second.end(), [&](const auto& obj) {
					return sessionID == obj.second;
				});
			if (subscriber_iter == iter->second.end()) {
				if (count == 0)
					return false;
				return true;
			}

			++count;
			iter->second.erase(subscriber_iter);
		}
	}

private:
	struct QData {
		std::string symbol;
		Subscribers subscribers;
		Orderbook orderbook;
	};

	Orderbooks orderbooks_{
		{"GBP/USD", {
			PriceSnapshot{BidOffer::Bid, 0.99, 3, "test::order::1"},
			PriceSnapshot{BidOffer::Offer, 1.01, 3, "test::order::1"},
			PriceSnapshot{BidOffer::Bid, 0.97, 3, "test::order::2"},
			PriceSnapshot{BidOffer::Offer, 1.03, 3, "test::order::2"},
			PriceSnapshot{BidOffer::Bid, 0.95, 3, "test::order::3"},
			PriceSnapshot{BidOffer::Offer, 1.05, 3, "test::order::3"}
		}}
	};
	Subscribed subscribed_;

	std::atomic<bool> stopped_;
	std::mutex mtx_;
	std::thread update_;
	std::thread worker_;
	moodycamel::ConcurrentQueue<std::shared_ptr<QData>> queue_;
	Callback publish_{};
};
