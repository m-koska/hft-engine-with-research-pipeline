#pragma once

#include <vector>
#include <cstdint>

#include "EngineStructures.hpp"
#include "Utils/HashMap.hpp"
#include "Utils/MemoryPool.hpp"

namespace Engine {
	/**
	 * @class OrderBook
	 * @brief Asymptotic $O(1)$ Limit Order Book with strict Bid/Ask memory isolation.
	 * @details This architecture is designed with real market realities in mind, where book crosses happen often.
	 */
	class OrderBook {
	private:
		/// @brief Pre-allocated memory pool for Order structures to avoid runtime heap allocation.
		Utils::MemoryPool<Order> order_pool;

		const size_t max_orders;
		const size_t max_price;

		/// @brief Flat arrays mapping discrete price ticks to Order Buckets. Isolated by side.
		std::vector<PriceLevelBucket> buy_price_levels;
		std::vector<PriceLevelBucket> sell_price_levels;

		/// @brief Level-2 Bitboards: 1 bit represents a single active price tick.
		std::vector<uint64_t> active_price_levels_buy;
		std::vector<uint64_t> summary_bitboard_buy;

		/// @brief Level-1 Bitboards: 1 bit summarises a 64-bit block of Level-2.
		std::vector<uint64_t> active_price_levels_sell;
		std::vector<uint64_t> summary_bitboard_sell;

		/// @brief Custom hash map for $O(1)$ order lookup via ITCH order_reference_number (order_id).
		Utils::HashMap<Order> order_map;

		uint32_t best_ask{UINT32_MAX};
		uint32_t best_bid{0};

		/**
		 * @brief Internal routine to safely unlink an Order from the doubly-linked list bucket and remove it from the map.
		 * @param current_order Pointer to the order being removed.
		 */
		void unlinkAndRemove(Order* current_order) noexcept;

	public:
		/**
		 * @brief Constructs the OrderBook and pre-allocates all necessary contiguous memory.
     * @param max_orders Maximum number of active orders supported by the memory pool and hash map.
     * @param max_price Maximum allowed price limit (in ticks) determining the flat array sizes.
     */
		explicit OrderBook(size_t max_orders, size_t max_price);

		/**
		 * @brief Adds a new limit order to the book and updates BBO if necessary.
		 * @param order_id Unique order reference number (from ITCH feed).
		 * @param price Limit price of the order in ticks.
		 * @param volume Number of shares.
		 * @param side Market side (`Side::BUY` or `Side::SELL`).
		 */
		void addOrder(uint64_t order_id, uint32_t price, uint32_t volume, Side side) noexcept;

		/**
		 * @brief Fully removes an order from the book and recomputes BBO via hardware bit-scan.
		 * @param order_id Unique order reference number.
		 */
		void removeOrder(uint64_t order_id) noexcept;

		/**
		 * @brief Partially reduces the volume of an existing order.
		 * @param order_id Unique order reference number.
		 * @param cancelled_volume Number of shares to subtract from the order and bucket.
		 * @note If cancelled_volume equals the order's total volume, it is equivalent to removeOrder.
		 */
		void reduceOrderVolume(uint64_t order_id, uint32_t cancelled_volume) noexcept;

		/**
		 * @brief Replaces an existing order with a new ID, price, and volume, maintaining side.
		 * @param old_id The reference number of the order to be replaced.
		 * @param new_id The new reference number assigned to the updated order.
		 * @param new_price The new limit price.
		 * @param new_volume The new share volume.
		 * @param side Market side (BUY or SELL).
		 */
		void replaceOrder(uint64_t old_id, uint64_t new_id, uint32_t new_price, uint32_t new_volume, Side side) noexcept;

		/**
		 * @brief Retrieves a raw pointer to an active order in $O(1)$ time.
		 * @param order_id Unique order reference number.
		 * @return Pointer to the Order, or nullptr if not found.
		 */
		[[nodiscard]] Order* getOrder(const uint64_t order_id) const noexcept {
			return order_map.get(order_id);
		}

		/**
		 * @brief Returns the current Best Bid (highest buy price).
		 * @return Best Bid price, or 0 if the buy book is empty.
		 */
		[[nodiscard]] uint32_t getBestBidPrice() const noexcept { return best_bid; }

		/**
		 * @brief Returns the current Best Ask (lowest sell price).
		 * @return Best Ask price, or UINT32_MAX if the sell book is empty.
		 */
		[[nodiscard]] uint32_t getBestAskPrice() const noexcept { return best_ask; }

		/**
		 * @brief Retrieves the entire price bucket containing aggregated volume and order queues.
		 * @param price The target price level.
		 * @param side The side of the book to query (BUY or SELL).
		 * @return Constant reference to the PriceLevelBucket.
		 */
		[[nodiscard]] const PriceLevelBucket& getPriceLevel(const uint32_t price, const Side side) const noexcept {
			return side == Side::BUY ? buy_price_levels[price] : sell_price_levels[price];
		}

	};

}