#include "..\..\include\mslvm\fwd.hpp"
#include "..\..\include\mslvm\vm_definitions.hpp"
#include "..\..\include\mslvm\vm_stack.hpp"
#include "..\..\include\mslvm\vm_memory.hpp"

namespace MSLVM
{
	void DynamicMemory::TransferData(uint8_t* new_m_pointer, size_t new_size)
	{
	}
	void DynamicMemory::IncreaseStack()
	{
	}
	void DynamicMemory::IncreaseHeap()
	{
	}
	void DynamicMemory::DencreaseStack()
	{
	}
	void DynamicMemory::DencreaseHeap()
	{
	}
	
	uint64_t DynamicMemory::Read(uint64_t address, size_t size)
	{
		uint64_t value = 0;
		size = size > VALID_MEMORY_ACCESS_SIZE ? VALID_MEMORY_ACCESS_SIZE : size;
		for (size_t i = 0; i < size; i++) {
			value |= static_cast<uint64_t>(m_pointer[address + i]) << (i * BYTE);
		}
		return value;
	}
	void DynamicMemory::Write(uint64_t address, uint64_t value, size_t size)
	{
		if (address < stack_sa && address + size > code_sa)
		{
			status = ErrorCode::ChangingConstantData;
			return;
		}
		size = size > VALID_MEMORY_ACCESS_SIZE ? VALID_MEMORY_ACCESS_SIZE : size;
		for (size_t i = 0; i < size; i++) {
			m_pointer[address + i] = static_cast<uint8_t>((value >> (i * BYTE)) & 0xFF);
		}
	}

	void DynamicMemory::Allocate(size_t code_size, size_t rod_size)
	{
		size = code_size + rod_size + MinStackSize + MinHeapSize;

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
	}
}