#pragma once

#include <cstdint>
#include <cstdlib>


// Intrusive list based memory pool
// More cache friendly than two list oriented approach
// Pointers and elements are stored in the same place in the memory

namespace Utils {

	template <typename T>
	class MemoryPool {

		private:
			const size_t pool_size;
			T* pool;
			T* free_element_list_head;

		public:
			explicit MemoryPool(const size_t pool_size) : pool_size(pool_size) {

				pool = static_cast<T*>(malloc(sizeof(T) * pool_size));
				free_element_list_head = &pool[0];
				for (size_t i = 0 ; i < pool_size-1; i++) {
					*reinterpret_cast<T**>(&pool[i]) = &pool[i + 1];
				}

				*reinterpret_cast<T**>(&pool[pool_size -1 ]) = nullptr;

				free_element_list_head = &pool[0];


			}

			T* allocate() {

				if (free_element_list_head == nullptr) {
					return nullptr;
				}

				T* element = free_element_list_head;
				free_element_list_head = *reinterpret_cast<T**>(free_element_list_head);
				return element;
			}

			void deallocate(T* element) {
				*reinterpret_cast<T**>(element) = free_element_list_head;
				free_element_list_head = element;
			}

			~MemoryPool() {
				free(pool);
			}

	};

}