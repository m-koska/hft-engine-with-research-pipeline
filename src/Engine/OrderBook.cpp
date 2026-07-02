#include "Engine/OrderBook.hpp"
#include "Utils/MemoryPool.hpp"

#include <bit>

namespace Engine {

	OrderBook::OrderBook(const size_t max_orders, const size_t max_order_id, const size_t max_price)
		:
		order_pool(max_orders),
		max_order_id(max_order_id),
		max_orders(max_orders),
		max_price(max_price) {

		price_level_bucket_pool = static_cast<PriceLevelBucket *>(
			calloc(max_price, sizeof(PriceLevelBucket)));

		active_price_levels = static_cast<uint64_t *>(
			calloc((max_price / 64) + 1, sizeof(uint64_t)));

		order_map = static_cast<Order **>(calloc(max_order_id, sizeof(Order *)));

		best_bid = 0;
		best_ask = UINT32_MAX;
	}

	OrderBook::~OrderBook() {
		free(price_level_bucket_pool);
		free(active_price_levels);
		free(order_map);
	}

	void OrderBook::addOrder(const uint64_t order_id, const uint32_t price, const uint32_t volume, const Side side) {

		if (order_id >= max_order_id || price >= max_price) {
			return;
		}

		Order* current_order = order_pool.allocate();
		if (current_order == nullptr) [[unlikely]] {
			return;
		}

		current_order->order_id = order_id;
		current_order->volume = volume;
		current_order->side = side;

		if (side == Side::BUY) {
			if (price > best_bid) {
				best_bid = price;
			}
		}
		if (side == Side::SELL) {
			if (price < best_ask) {
				best_ask = price;
			}
		}

		PriceLevelBucket* current_price_level = &price_level_bucket_pool[price];
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
			const uint32_t price_block_index = price >> 6; // price/64 (2^6)
			const uint32_t price_bit_index = price & 0x3F; // price%64

			active_price_levels[price_block_index] |= (1ULL << price_bit_index);

		}

		//trzeba to zmodyfikować potem, bo id na Nasdaq to są ogromne liczby
		order_map[order_id] = current_order;

	}

	void OrderBook::removeOrder(const uint64_t order_id) {

		const auto current_order = getOrder(order_id);
		unlinkAndRemove(current_order);

	}

	void OrderBook::reduceOrderVolume(const uint64_t order_id, uint32_t cancelled_volume) {

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

	void OrderBook::replaceOrder(const uint64_t old_id, const uint64_t new_id, const uint32_t new_price, const uint32_t new_volume, const Side side) {
		removeOrder(old_id);
		addOrder(new_id, new_price, new_volume, side);
	}

	Order* OrderBook::getOrder(const uint64_t order_id) noexcept {

		if (order_id > max_order_id-1) {
			return nullptr;
		}

		return order_map[order_id];

	}

	inline void OrderBook::unlinkAndRemove(Order* current_order) {

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

			active_price_levels[price_block_index] &= ~(1ULL << price_bit_index);

			if (current_order->side == Side::BUY) {

				if (current_price_level->price == best_bid) {

					best_bid = 0;

					auto block_index = static_cast<int32_t>(price_block_index);

					while (block_index >= 0) {

						const uint64_t& current_block = active_price_levels[block_index];

						if (current_block == 0) {
							block_index--;
						} else {
							best_bid =
								(block_index << 6)
								+ (0x3F - std::countl_zero(current_block));
							break;
						}

					}

				}

			}

			if (current_order->side == Side::SELL) {

				if (current_price_level->price == best_ask) {

					best_ask = UINT32_MAX;

					auto block_index = static_cast<int32_t>(price_block_index);

					while (block_index <= static_cast<int32_t>(max_price >> 6)) {

						const uint64_t& current_block = active_price_levels[block_index];

						if (current_block == 0) {
							block_index++;
						} else {
							best_ask = (block_index << 6)
								+ std::countr_zero(current_block);
							break;
						}

					}

				}

			}

		}

		order_pool.deallocate(current_order);
		order_map[order_id] = nullptr;
	}

	const uint32_t OrderBook::getBestBidPrice() const {
		return this->best_bid;
	}

	const uint32_t OrderBook::getBestAskPrice() const {
		return this->best_ask;
	}

	const PriceLevelBucket& OrderBook::getPriceLevel(const uint32_t price) const {
		return this->price_level_bucket_pool[price];
	}
}
