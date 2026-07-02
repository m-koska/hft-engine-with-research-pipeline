#pragma once
#include <cstdint>

namespace Engine {
	/// @brief Represents the side of the market for a given action or order.
	enum class Side : uint8_t {
		BUY = 'B',
		SELL = 'S'
	};

	// forward declaration
	struct Order;

	/// @brief Represents a single price level in the order book.
	/// @details contains the pointers to the first and last element of the intrusive list of @ref Order .
	struct PriceLevelBucket {
		Order* first_order{nullptr};
		Order* last_order{nullptr};

		uint32_t price{0};
		uint32_t total_volume{0};
	};

	/// @brief Represents a single order on a price level in the order book
	/// @details An element of @ref PriceLevelBucket list
	struct Order {
		// Pointer to price level
		PriceLevelBucket* price_level_bucket{nullptr};

		// Next order in the list (lower priority)
		Order* next{nullptr};
		// Previous order in the list (higher priority)
		Order* prev{nullptr};

		// Unique order ID
		uint64_t order_id{0};

		// Order volume
		uint32_t volume{0};
		// Side of the Order
		Side side{Side::BUY};
	};

}
