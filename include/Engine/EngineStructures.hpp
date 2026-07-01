#pragma once
#include <cstdint>
namespace Engine {

	enum class Side : uint8_t {
		BUY = 'B',
		SELL = 'S'
	};

	struct Order;

	struct PriceLevelBucket {
		Order* first_order;
		Order* last_order;

		uint32_t price;
		uint32_t total_volume;
		// skoro PriceLevelBucket będzie w płaskiej tablicy, to nie potrzebuję trzymać w nim wskaźników na najbliższe ceny
	};

	struct Order {
		PriceLevelBucket* price_level_bucket;

		Order* next;
		Order* prev;

		uint64_t order_id;

		uint32_t volume;
		Side side;
	};

}
