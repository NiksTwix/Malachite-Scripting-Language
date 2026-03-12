#pragma once
#include "vm_errors.hpp"
#include "vm_memory.hpp"

#define REG_U(reg) ((reg).u)	   // для номеров регистров
#define REG_I(reg) ((reg).i)		// для immediate integer
#define REG_R(reg) ((reg).r)		// для immediate real


namespace MSLVM
{
	struct alignas(8) LinearMemory
	{
		memory_cell memory[STACK_SIZE + HEAP_SIZE]{};
		HeapFreeIntervals HFI{HEAP_START, HEAP_END};

		size_t rod_size = 0;
		uint8_t* rod_memory = nullptr;		//Allocates by loader
	};

		   



	
	struct CallFrame
	{
		uint64_t sp;
		uint64_t fp;
		uint64_t ip;
		CallFrame() = default;
		CallFrame(uint64_t stack_pointer, uint64_t frame_pointer, uint64_t instruction_pointer) : sp(stack_pointer), fp(frame_pointer), ip(instruction_pointer) {}
	};

	struct VMState
	{
		Register registers[TOTAL_REGISTERS]{};		//All here: flags, special register and etc
		LinearMemory memory{};
		CallStack call_stack{};
		ErrorStack error_stack{};

		VMState()
		{
			registers[SpecialRegister::IP].u = 0;
			registers[SpecialRegister::SP].u = STACK_START;
			registers[SpecialRegister::FP].u = STACK_START;
			registers[SpecialRegister::FL].u = 0;

		}

	};

}