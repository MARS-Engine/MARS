#pragma once

#include <mars/debug/logger.hpp>

#include <new>
#include <stddef.h>

namespace mars {

inline const log_channel& array_stack_heap_allocator_channel() {
	static const log_channel channel("array_stack_heap_allocator");
	return channel;
}

template <size_t capacity>
class array_stack_heap_allocator {
  public:
	array_stack_heap_allocator()
		: blocks(), count(0) {
	}

	~array_stack_heap_allocator() {
		for (size_t i = count; i > 0; --i)
			::operator delete(blocks[i - 1]);
	}

	template <typename type>
	type* allocate(size_t element_count) {
		logger::assert_(count < capacity, array_stack_heap_allocator_channel(), "array_stack_heap_allocator capacity exceeded: {} >= {}", count, capacity);
		size_t byte_count = element_count > size_t(-1) / sizeof(type) ? size_t(-1) : element_count * sizeof(type);
		type* result = static_cast<type*>(::operator new(byte_count));
		blocks[count++] = result;
		return result;
	}

	void deallocate(void* ptr) {
		logger::assert_(count > 0, array_stack_heap_allocator_channel(), "array_stack_heap_allocator deallocate called with empty stack");
		logger::assert_(blocks[count - 1] == ptr, array_stack_heap_allocator_channel(), "array_stack_heap_allocator deallocate order violation");
		::operator delete(ptr);
		count--;
	}

  private:
	void* blocks[capacity];
	size_t count;
};

} // namespace mars
