#pragma once
#include <cstdint>
namespace Engine {

	enum class Side : uint8_t {
		BUY = 'B',
		SELL = 'S'
	};

	struct Order;

	struct PriceLevelBucket {
		Order* first_order{nullptr};
		Order* last_order{nullptr};

		uint32_t price{0};
		uint32_t total_volume{0};
	};

	struct Order {
		PriceLevelBucket* price_level_bucket{nullptr};

		Order* next{nullptr};
		Order* prev{nullptr};

		uint64_t order_id{0};

		uint32_t volume{0};
		Side side{Side::BUY};
	};

}
