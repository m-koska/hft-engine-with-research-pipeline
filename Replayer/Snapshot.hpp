#pragma once
#include <cstdint>

namespace Replayer {

	struct Snapshot {
		uint64_t parsed_timestamp;
		int64_t cumulated_ofi;
		double mid_price;
		double mid_price_delta;

		uint16_t stock_locate;
	};

	struct TickerMicrostate {
		int64_t current_ofi_accumulator = 0;
		double prev_snapshot_mid_price = 0.0;
		uint32_t prev_bid = 0;
		uint32_t prev_ask = UINT32_MAX;
		uint32_t prev_bid_vol = 0;
		uint32_t prev_ask_vol = 0;

		uint32_t event_count = 0;

		uint16_t stock_locate;
	};

}
