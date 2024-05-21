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

template <typename CALLER>
class MDPublisher {
public:
	using Orderbook   = std::vector<PriceSnapshot>;
	using Orderbooks  = std::unordered_map<std::string, Orderbook>;
	using Subscriber  = std::string; // std::string.toStringFrozen()
	using Subscribers = std::unordered_set<Subscriber>;
	using Subscribed  = std::unordered_map<std::string, Subscribers>;
	using Callback    = std::function<void(CALLER&, std::string, Subscribers, Orderbook)>;

	explicit MDPublisher(CALLER* obj = nullptr, Callback publish = nullptr) : target_(obj), publish_(publish) {
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
						if (target_ && publish_) {
							publish_(*target_, std::move(qdata->symbol, std::move(qdata->subscribers), std::move(qdata->orderbook)));
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
	struct QData {
		std::string symbol;
		Subscribers subscribers;
		Orderbook orderbook;
	};

	Orderbooks orderbooks_;
	Subscribed subscribed_;

	std::atomic<bool> stopped_;
	std::mutex mtx_;
	std::thread update_;
	std::thread worker_;
	moodycamel::ConcurrentQueue<std::shared_ptr<QData>> queue_;

	CALLER* target_{};
	Callback publish_{};
};
