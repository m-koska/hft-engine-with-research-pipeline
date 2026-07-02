# Changelog

### Added
- **Open Addressing Hash Map**: Custom `HashMap` implementation utilizing Fibonacci Hashing and linear probing to replace the 16GB flat pointer array.

### Changed
- **Memory Pool Alignment**: Transitioned the intrusive free-list mechanism to a Tagged Union (`union alignas(T) Slot`) to eliminate Strict Aliasing violations and guarantee cache-line alignment without `reinterpret_cast<void**>`.
- **Wire Protocol Parsers**: Refactored ITCH timestamp parser to a header-only
- **Directory Structure**: Migrated to canonical C++ layout separating public headers (`include/`) from implementation (`src/`).
- **C-Style Memory Management**: Eliminated unsafe `calloc`/`free` calls and raw pointers in `OrderBook`. Replaced with pre-allocated `std::vector` containers to enforce RAII
- **`HashMap` instead of flat array for mapping orders**: while it drastically decreased benchmark performance (designed for nicely incrementing IDs), it has proven a little more efficient on real-world data, which is the ultimate goal.

### Fixed
- **Dual-sided architecture Order Book**: fixed a critical bug, which was creating statistical artefacts every time the book was crossed, thanks to implementing separate price buckets for `SIDE::BUY` and `SIDE::SELL` 