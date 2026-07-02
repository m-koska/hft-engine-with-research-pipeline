#pragma once

#include <cstdint>

namespace Parser {
	/// @brief Converts a 6-byte Big-Endian NASDAQ ITCH timestamp to 64-bit integer.
	/// @param ts Pointer to exactly 6 bytes representing the timestamp.
	/// @return Number of nanoseconds since midnight.
	[[nodiscard]] inline uint64_t parseTimestamp(const uint8_t ts[6]) {
		return (static_cast<uint64_t>(ts[0]) << 40) |
			(static_cast<uint64_t>(ts[1]) << 32) |
			(static_cast<uint64_t>(ts[2]) << 24) |
			(static_cast<uint64_t>(ts[3]) << 16) |
			(static_cast<uint64_t>(ts[4]) << 8)  |
			(static_cast<uint64_t>(ts[5]));
	}
}