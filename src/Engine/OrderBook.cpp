#include "Engine/OrderBook.hpp"
#include "Utils/MemoryPool.hpp"

#include <bit>
#include <stdexcept>

namespace Engine {

	OrderBook::OrderBook(const size_t max_orders, const size_t max_price)
		: order_pool(max_orders),
			max_orders(max_orders),
			max_price(max_price),
			buy_price_levels(max_price),
			sell_price_levels(max_price),
			active_price_levels_buy((max_price >> 6) + 1, 0),
			summary_bitboard_buy((active_price_levels_buy.size() >> 6) + 1, 0),
			active_price_levels_sell((max_price >> 6) + 1, 0),
			summary_bitboard_sell((active_price_levels_sell.size() >> 6) + 1, 0),
			order_map(max_orders) {}

	void OrderBook::addOrder(const uint64_t order_id, const uint32_t price, const uint32_t volume, const Side side) noexcept {

		if (price >= max_price) [[unlikely]] {
			//throw std::runtime_error("CRITICAL: Price exceeding max_price" + std::to_string(price) + " exceeded max_price " + std::to_string(max_price));
			return;
		}

		Order* current_order = order_pool.allocate();
		if (current_order == nullptr) [[unlikely]] {
			throw std::runtime_error("CRITICAL: Memory Pool exhausted! Increase max_orders.");
			return;
		}

		current_order->order_id = order_id;
		current_order->volume = volume;
		current_order->side = side;

		PriceLevelBucket* current_price_level = nullptr;

		if (side == Side::BUY) {

			if (price > best_bid) best_bid = price;

			current_price_level = &buy_price_levels[price];

		}
		else {

			if (price < best_ask) best_ask = price;

			current_price_level = &sell_price_levels[price];

		}

		current_order->price_level_bucket = current_price_level;
		current_price_level->price = price;
		current_price_level->total_volume += volume;

		const auto current_last = current_price_level->last_order;
		const auto current_first = current_price_level->first_order;

		current_order->prev = current_last;
		current_order->next = nullptr;

		if (current_last != nullptr) {
			current_last->next = current_order;
		}
		current_price_level->last_order = current_order;

		if (current_first == nullptr) {

			current_price_level->first_order = current_order;

			const uint32_t price_block_index = price >> 6;
			const uint32_t price_bit_index = price & 0x3F;
			const uint32_t summary_block_index = price_block_index >> 6;
			const uint32_t summary_bit_index = price_block_index & 0x3F;

			if (side == Side::BUY) {
				active_price_levels_buy[price_block_index] |= (1ULL << price_bit_index);
				summary_bitboard_buy[summary_block_index] |= (1ULL << summary_bit_index);
			} else {
				active_price_levels_sell[price_block_index] |= (1ULL << price_bit_index);
				summary_bitboard_sell[summary_block_index] |= (1ULL << summary_bit_index);
			}

		}

		order_map.insert(order_id, current_order);

	}

	void OrderBook::removeOrder(const uint64_t order_id) noexcept {

		const auto current_order = getOrder(order_id);
		unlinkAndRemove(current_order);

	}

	void OrderBook::reduceOrderVolume(const uint64_t order_id, uint32_t cancelled_volume) noexcept {

		const auto current_order = getOrder(order_id);
		if (current_order == nullptr) {
			return;
		}

		const auto current_price_level = current_order->price_level_bucket;

		if (cancelled_volume > current_order->volume) {
			cancelled_volume = current_order->volume;
		}

		if (current_order->volume == cancelled_volume) {
			unlinkAndRemove(current_order);
			return;
		}

		current_price_level->total_volume -= cancelled_volume;
		current_order->volume -= cancelled_volume;

	}

	void OrderBook::replaceOrder(const uint64_t old_id, const uint64_t new_id, const uint32_t new_price, const uint32_t new_volume, const Side side) noexcept {
		removeOrder(old_id);
		addOrder(new_id, new_price, new_volume, side);
	}

	inline void OrderBook::unlinkAndRemove(Order* current_order) noexcept {

		if (current_order == nullptr) {
			return;
		}

		const uint64_t order_id = current_order->order_id;
		const auto current_price_level = current_order->price_level_bucket;

		current_price_level->total_volume -= current_order->volume;

		const auto current_prev = current_order->prev;
		const auto current_next = current_order->next;

		if (current_prev != nullptr) {
			current_prev->next = current_next;
		} else {
			current_price_level->first_order = current_next;
		}
		if (current_next != nullptr) {
			current_next->prev = current_prev;
		} else {
			current_price_level->last_order = current_prev;
		}

		if (current_price_level->total_volume == 0) {

			const auto price = current_price_level->price;
			const uint32_t price_bit_index = price & 0x3F;
			const uint32_t price_block_index = price >> 6;

			if (current_order->side == Side::BUY) {
			    active_price_levels_buy[price_block_index] &= ~(1ULL << price_bit_index);

			    if (active_price_levels_buy[price_block_index] == 0) {
			        const uint32_t summary_block_index = price_block_index >> 6;
			        const uint32_t summary_bit_index = price_block_index & 0x3F;
			        summary_bitboard_buy[summary_block_index] &= ~(1ULL << summary_bit_index);
			    }

			    if (price == best_bid) {
			        best_bid = 0;
			        auto summary_idx = static_cast<int32_t>(price_block_index >> 6);
			        while (summary_idx >= 0) {
			            if (summary_bitboard_buy[summary_idx] != 0) {
			                const uint32_t block_offset = 63 - std::countl_zero(summary_bitboard_buy[summary_idx]);
			                const uint32_t block_idx = (summary_idx << 6) + block_offset;
			                best_bid = (block_idx << 6) + (63 - std::countl_zero(active_price_levels_buy[block_idx]));
			                break;
			            }
			            summary_idx--;
			        }
			    }
			} else {
			    active_price_levels_sell[price_block_index] &= ~(1ULL << price_bit_index);

			    if (active_price_levels_sell[price_block_index] == 0) {
			        const uint32_t summary_block_index = price_block_index >> 6;
			        const uint32_t summary_bit_index = price_block_index & 0x3F;
			        summary_bitboard_sell[summary_block_index] &= ~(1ULL << summary_bit_index);
			    }

			    if (price == best_ask) {
			        best_ask = UINT32_MAX;
			        auto summary_idx = static_cast<size_t>(price_block_index >> 6);
			        while (summary_idx < summary_bitboard_sell.size()) {
			            if (summary_bitboard_sell[summary_idx] != 0) {
			                const uint32_t block_offset = std::countr_zero(summary_bitboard_sell[summary_idx]);
			                const uint32_t block_idx = (summary_idx << 6) + block_offset;
			                best_ask = (block_idx << 6) + std::countr_zero(active_price_levels_sell[block_idx]);
			                break;
			            }
			            summary_idx++;
			        }
			    }
			}

		}

		order_pool.deallocate(current_order);
		order_map.erase(order_id);
	}

}
