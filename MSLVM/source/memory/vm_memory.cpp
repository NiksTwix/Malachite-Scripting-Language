#include "..\..\include\mslvm\fwd.hpp"
#include "..\..\include\mslvm\vm_definitions.hpp"
#include "..\..\include\mslvm\vm_stack.hpp"
#include "..\..\include\mslvm\vm_memory.hpp"

namespace MSLVM
{
	void DynamicMemory::TransferData(size_t new_stack_size, size_t new_heap_size)
	{
		size_t new_size = code_size + rod_size + new_stack_size + new_heap_size;
		auto pointer_p = static_cast<uint8_t*>(calloc(new_size, sizeof(uint8_t)));
		if (pointer_p == nullptr)
		{
			status = ErrorCode::FailedMemoryAllocation;
			return;
		}

		memcpy(pointer_p, m_pointer, code_size + rod_size);		//copieng rod and code

		memcpy(pointer_p + stack_sa, m_pointer + stack_sa, stack_size);

		memcpy(pointer_p + (stack_sa + new_stack_size), m_pointer + heap_sa, heap_size);

		heap_sa = stack_sa + new_stack_size;


		free(m_pointer);
		m_pointer = pointer_p;
		stack_size = new_stack_size;
		heap_size = new_heap_size;
		size = new_size;
	}

	void DynamicMemory::IncreaseHeap()
	{
		TransferData(stack_size, heap_size * 2);
	}

	void DynamicMemory::DencreaseHeap()
	{
	}
	
	uint64_t DynamicMemory::Read(uint64_t address, size_t size)
	{
		uint64_t native_address = rod_sa + address;
		uint64_t value = 0;
		size = size > VALID_MEMORY_ACCESS_SIZE ? VALID_MEMORY_ACCESS_SIZE : size;
		for (size_t i = 0; i < size; i++) {
			value |= static_cast<uint64_t>(m_pointer[native_address + i]) << (i * BYTE);
		}
		return value;
	}
	void DynamicMemory::Write(uint64_t address, uint64_t value, size_t size)
	{
		uint64_t native_address = rod_sa + address;
		if (native_address < stack_sa)
		{
			status = ErrorCode::ChangingConstantData;
			return;
		}
		size = size > VALID_MEMORY_ACCESS_SIZE ? VALID_MEMORY_ACCESS_SIZE : size;
		for (size_t i = 0; i < size; i++) {
			m_pointer[native_address + i] = static_cast<uint8_t>((value >> (i * BYTE)) & 0xFF);
		}
	}

	VMOperation& DynamicMemory::GetOperation(size_t index)
	{
		return *(reinterpret_cast<VMOperation*>(m_pointer) + index);
	}

	void DynamicMemory::Allocate(size_t code_size, size_t rod_size)
	{
		//Align for aligned heap and stack
		code_size = ((code_size + DEFAULT_ALIGNMENT - 1) / DEFAULT_ALIGNMENT) * DEFAULT_ALIGNMENT;
		rod_size = ((rod_size + DEFAULT_ALIGNMENT - 1) / DEFAULT_ALIGNMENT) * DEFAULT_ALIGNMENT;

		size = code_size + rod_size + MinStackSize + MinHeapSize;

		this->code_size = code_size;
		this->rod_size = rod_size;

		m_pointer = static_cast<uint8_t*>(calloc(size, sizeof(uint8_t)));

		if (m_pointer == nullptr)
		{
			status = ErrorCode::FailedMemoryAllocation;
			return;
		}

		code_sa = 0;

		rod_sa = code_size;
		stack_sa = rod_sa + rod_size;
		heap_sa = stack_sa + MinStackSize;

		stack_size = MinStackSize;
		heap_size = MinHeapSize;

		end = size;
	}

	void DynamicMemory::Free()
	{
		if (m_pointer == nullptr)
		{
			status = ErrorCode::FailedMemoryFreeing;
			return;
		}

		free(m_pointer);

		code_sa = 0;
		rod_sa = 0;
		stack_sa = 0;
		heap_sa = 0;
		end = 0;
		code_size = 0;
		rod_size = 0;
		stack_size = 0;
		heap_size = 0;
	}
	void HeapFreeIntervals::AddInterval(const HeapFreeInterval& interval)
	{
		// Добавить в список по адресу (отсортированно)
		free_intervals_by_addr.insert_sorted(interval,
			[](const HeapFreeInterval& a, const HeapFreeInterval& b) {
				return a.start < b.start;
			});

		//  Append in index by size
		free_intervals_by_size[interval.size].push_back(interval);

		// Update min/max
		if (interval.size < min_block_size) min_block_size = interval.size;
		if (interval.size > max_block_size) max_block_size = interval.size;
	}
	void HeapFreeIntervals::RemoveInterval(const HeapFreeInterval& interval)
	{
		// Find and remove from list by address
		auto node = free_intervals_by_addr.find_if(
			[&](const HeapFreeInterval& block) {
				return block.start == interval.start &&
					block.end == interval.end;
			});
		if (node) {
			free_intervals_by_addr.remove(node);
		}

		// Remove from index by size
		auto it = free_intervals_by_size.find(interval.size);
		if (it != free_intervals_by_size.end()) {
			auto& list = it->second;
			auto size_node = list.find_if(
				[&](const HeapFreeInterval& block) {
					return block.start == interval.start;
				});
			if (size_node) {
				list.remove(size_node);
			}

			// If list began empty -> delete writting
			if (list.empty()) {
				free_intervals_by_size.erase(it);
			}
		}

		
	}

	std::shared_ptr<typename LinkedList<HeapFreeInterval>::LinkedNode> HeapFreeIntervals::FindFitBlock(uint64_t required_size, HeapFreeInterval* out_block)
	{
		// Search nearest necessary size
		for (size_t size = required_size; size <= max_block_size; ++size) {
			auto it = free_intervals_by_size.find(size);
			if (it != free_intervals_by_size.end() && !it->second.empty()) {
				// Take first block by this size
				auto node = it->second.get_front();
				if (out_block) {
					*out_block = node->data;
				}
				return node;
			}
		}
		return nullptr;
	}
	ErrorCode HeapFreeIntervals::Allocate(uint64_t size, uint64_t* address)
	{
		if (size == 0 || address == nullptr) {
			return ErrorCode::FailedMemoryAllocation;
		}

		// Align the size
		uint64_t aligned_size = ((size + alignment - 1) / alignment) * alignment;

		// Find necessary block
		HeapFreeInterval block;
		auto node = FindFitBlock(aligned_size, &block);
		if (node == nullptr) {
			return ErrorCode::FailedMemoryAllocation; // Necessary block doesnt exist
		}

		*address = block.start;

		// Split block
		HeapFreeInterval allocated_part, remaining_part;
		int has_remaining = block.Split(aligned_size, &allocated_part, &remaining_part);

		// Remove old block from indices
		RemoveInterval(block);

		// If remainder exists -> append it back
		if (has_remaining) {
			AddInterval(remaining_part);
		}

		return ErrorCode::NoError;
	}

	ErrorCode HeapFreeIntervals::AllocateAligned(uint64_t size, size_t specific_alignment, uint64_t* address)
	{
		if (size == 0 || address == nullptr) return ErrorCode::FailedMemoryAllocation;

		size_t actual_alignment = alignment > specific_alignment ? alignment : specific_alignment;

		// Go by all blocks, find necessary with alignment
		auto node = free_intervals_by_addr.get_front();
		while (node) {
			uint64_t block_start = node->data.start;
			uint64_t block_size = node->data.size;

			// Calculating the aligned address within a block 
			uint64_t aligned_start = ((block_start + actual_alignment - 1)
				/ actual_alignment) * actual_alignment;

			// We check if the block fits with this alignment.
			if (aligned_start + size - 1 <= node->data.end) {
				uint64_t required_total_size = aligned_start - block_start + size;

				if (block_size >= required_total_size) {
					*address = aligned_start;

					// Remove block from indices
					RemoveInterval(node->data);

					// Create blocks for part before and after aligned area
					if (aligned_start > block_start) {
						// Front part
						HeapFreeInterval front_block(block_start, aligned_start - 1);
						AddInterval(front_block);
					}

					if (aligned_start + size - 1 < node->data.end) {
						// Back part
						HeapFreeInterval back_block(aligned_start + size, node->data.end);
						AddInterval(back_block);
					}

					// Merge adjacent (смежные) blocks
					Coalesce();

					return ErrorCode::NoError;
				}
			}

			node = node->next;
		}

		return ErrorCode::FailedMemoryAllocation;
	}

	ErrorCode HeapFreeIntervals::Free(uint64_t address, uint64_t size)
	{
		if (size == 0) return ErrorCode::FailedMemoryFreeing;

		// Align size
		uint64_t aligned_size = ((size + alignment - 1) / alignment) * alignment;
		uint64_t end = address + aligned_size - 1;

		// Checking of exit beyond heap borders
		if (end < address) return ErrorCode::FailedMemoryFreeing;// Overflow

		// Create new free block
		HeapFreeInterval free_block(address, end);

		// Append it in indices
		AddInterval(free_block);

		// Merge adjacent blocks
		Coalesce();

		return ErrorCode::NoError;
	}

	void HeapFreeIntervals::RebuildSizeIndex()
	{
		free_intervals_by_size.clear();
		min_block_size = SIZE_MAX;
		max_block_size = 0;

		auto node = free_intervals_by_addr.get_front();
		while (node) {
			free_intervals_by_size[node->data.size].push_back(node->data);

			if (node->data.size < min_block_size) min_block_size = node->data.size;
			if (node->data.size > max_block_size) max_block_size = node->data.size;

			node = node->next;
		}
	}


	

}