#pragma once

#include <cstdlib>


// Intrusive list based memory pool
// More cache friendly than two list oriented approach
// Pointers and elements are stored in the same place in the memory

namespace Utils {

	template <typename T>
	class MemoryPool {
		// safety-check to avoid UB
		static_assert(sizeof(T) >= sizeof(void*), "Type size must be at least pointer size for intrusive list.");

		private:

		union alignas(T) Slot { // Avoids using reinterpret_cast between storage and free-list node
			T element;						// still might be worth to test performance with std::variant to ensure max possible safety
			Slot* next_free;
		};

		const size_t pool_size;
		Slot* pool{nullptr};
		Slot* free_element_list_head{nullptr};

		public:

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

		[[nodiscard]] T* allocate() {

			if (free_element_list_head == nullptr) [[unlikely]] {
				return nullptr;
			}

			Slot* allocated_slot = free_element_list_head;
			free_element_list_head = allocated_slot->next_free;
			return &allocated_slot->element;

		}

		void deallocate(T* element) {

			if (!element) [[unlikely]] return;

			Slot* slot = reinterpret_cast<Slot*>(element);
			slot->next_free = free_element_list_head;
			free_element_list_head = slot;

		}

	};

}