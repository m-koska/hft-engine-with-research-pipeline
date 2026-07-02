# Changelog

### Added
- **Open Addressing Hash Map**: Custom `HashMap` implementation utilizing Fibonacci Hashing and linear probing to replace the 16GB flat pointer array.

### Changed
- **Memory Pool Alignment**: Transitioned the intrusive free-list mechanism to a Tagged Union (`union alignas(T) Slot`) to eliminate Strict Aliasing violations and guarantee cache-line alignment without `reinterpret_cast<void**>`.
- **Wire Protocol Parsers**: Refactored ITCH timestamp parser to a header-only
- **Directory Structure**: Migrated to canonical C++ layout separating public headers (`include/`) from implementation (`src/`).