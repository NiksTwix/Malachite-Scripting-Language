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
}