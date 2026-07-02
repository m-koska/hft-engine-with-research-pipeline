#pragma once

#include <cstdlib>

namespace Utils {

	/// @class MemoryPool
	 /// @brief Intrusive list based memory pool.
	 /// @details More cache friendly than two list oriented approach since
	 /// pointers and elements are stored in the same place in the memory.
	 /// @tparam T The type of elements to be stored in the pool, where sizeof(T) >= sizeof(void*) [size of a pointer].
	template <typename T>
	class MemoryPool {
		// safety-check to avoid UB
		static_assert(sizeof(T) >= sizeof(void*), "Type size must be at least pointer size for intrusive list.");

		private:
		/// @union Slot
		/// @brief Contains either an element of type T or pointer to the next free element.
		union alignas(T) Slot { // Avoids using reinterpret_cast between storage and free-list node
			T element;						// still might be worth to test performance with std::variant to ensure max possible safety
			Slot* next_free;
		};

		const size_t pool_size;
		Slot* pool{nullptr};
		Slot* free_element_list_head{nullptr};

		public:
		/// @brief Initialise the memory pool.
		/// /// @param pool_size Maximum number of the elements in the pool
		explicit MemoryPool(const size_t pool_size) : pool_size(pool_size) {

			pool = static_cast<Slot*>(std::aligned_alloc(alignof(Slot), sizeof(Slot) * pool_size));
			free_element_list_head = &pool[0];

			for (size_t i = 0 ; i < pool_size-1; i++) {
				pool[i].next_free = &pool[i + 1];
			}
			pool[pool_size - 1].next_free = nullptr;

			free_element_list_head = &pool[0];

		}

		~MemoryPool() {
				free(pool);
		}

		/// @brief Allocates a single element from the pool.
		/// @return Pointer to the allocated element, or nullptr if the pool is exhausted.
		[[nodiscard]] T* allocate() {

			if (free_element_list_head == nullptr) [[unlikely]] {
				return nullptr;
			}

			Slot* allocated_slot = free_element_list_head;
			free_element_list_head = allocated_slot->next_free;
			return &allocated_slot->element;

		}

		/// @brief Returns an element back to the pool.
		/// @param element Pointer to the element to deallocate.
		void deallocate(T* element) {

			if (!element) [[unlikely]] return;

			Slot* slot = reinterpret_cast<Slot*>(element);
			slot->next_free = free_element_list_head;
			free_element_list_head = slot;

		}

	};

}