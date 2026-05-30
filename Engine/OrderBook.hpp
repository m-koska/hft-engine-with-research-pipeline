#pragma once

#include "EngineStructures.hpp"
#include "../Utils/MemoryPool.hpp"

namespace Engine {

	class OrderBook {
	private:
		Utils::MemoryPool<Order> order_pool;

		PriceLevelBucket* price_level_bucket_pool;
		// each 64bit block represents 64 price levels
		uint64_t* active_price_levels;
		Order** order_map;

		const size_t max_order_id;
		const size_t max_orders;
		const size_t max_price;

		uint32_t best_ask;
		uint32_t best_bid;

	public:
		explicit OrderBook(size_t max_orders, size_t max_order_id, size_t max_price);

		~OrderBook();

		void addOrder(uint64_t order_id, uint32_t price, uint32_t volume, Side side);

		void removeOrder(uint64_t order_id);

		void reduceOrderVolume(uint64_t order_id, uint32_t cancelled_volume);

		void replaceOrder(uint64_t old_id, uint64_t new_id, uint32_t new_price, uint32_t new_volume, Side side);

		[[nodiscard]] Order* getOrder(uint64_t order_id) noexcept;

		inline void unlinkAndRemove(Order* current_order);

		const uint32_t getBestBidPrice() const;

		const uint32_t getBestAskPrice() const;

		const PriceLevelBucket& getPriceLevel(uint32_t price) const;

	};

}