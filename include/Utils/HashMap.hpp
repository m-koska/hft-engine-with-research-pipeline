#pragma once

#include <cstdint>
#include <cstdlib>
#include <vector>

namespace Utils {

  /// @class HashMap
  /// @brief Zero-allocation hashmap
  /// @details Fibonacci hashing etc. finish docs later
  /// @tparam T The type of elements of the map.
  template<typename T>
  class HashMap {

  private:
    /// @union Slot
    /// @brief Contains key and pointer to the element
    struct Slot {
      uint64_t key;
      T* value;
    };

    // for the hash function
    static constexpr uint64_t GOLDEN_RATIO = 11400714819323198485ULL;

    T* const TOMBSTONE = reinterpret_cast<T*>(-1);

    std::vector<Slot> map;
    const size_t capacity;
    const size_t mask;
    const uint32_t shift;

    /// @brief Performs Fibonacci hashing.
    /// @param key Key to be hashed
    [[nodiscard]] inline size_t hash(uint64_t key) const noexcept {
      return (GOLDEN_RATIO * key) >> shift;
    }

  public:
    /// @brief Initialise the map.
    /// @param max_elements Maximum number of the elements stored in the map.
    explicit HashMap(size_t max_elements)
      : capacity(std::bit_ceil(max_elements)),
        mask(capacity - 1),
        shift(64 - std::countr_zero(capacity)),
        map(capacity, {0, nullptr}) {}

    /// @brief Inserts a new element.
    /// @param key The key of the element.
    /// @param value Pointer to the element
    void insert(const uint64_t key, const T* value) {

      size_t index = hash(key);
      while (map[index].value != nullptr && map[index].key != TOMBSTONE) {
        index = (index +1) % capacity; // improved readibility, compilers makes it into (index+1) & mask anyways
      }
      map[index].key = key;
      map[index].value = value;

    }

    /// @brief Grants access to an element by its key.
    /// @param key The key of the element.
    /// @return Either a pointer to the element or nullptr.
    [[nodiscard]] T* get(const uint64_t key) {

      size_t index = hash(key);

      while (map[index].value != nullptr) {

        if (map[index].value != TOMBSTONE && map[index].key == key) [[likely]] {
          return map[index].value;
        }

        index = (index+1) % capacity;

      }

      return nullptr;

    }

    /// @brief Erases an element from the map.
    /// @param key The key of the element.
    void erase(uint64_t key) {

      size_t index = hash(key);

      while (map[index].value != nullptr) {

        if (map[index].value != TOMBSTONE && map[index].key == key) [[likely]] {
          map[index].value = TOMBSTONE;
        }

        index = (index+1) % capacity;

      }

    }

  };

}
