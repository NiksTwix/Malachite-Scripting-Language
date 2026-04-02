#pragma once
#include "vm_memory.hpp"
#include "fwd.hpp"
#define REG_U(reg) ((reg).u)	   // для номеров регистров
#define REG_I(reg) ((reg).i)		// для immediate integer
#define REG_R(reg) ((reg).r)		// для immediate real


namespace MSLVM
{
	
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
		DynamicMemory memory{};

		HeapFreeIntervals HFI{};

		CallStack call_stack{};
		ErrorStack error_stack{};

		VMState()
		{
			registers[SpecialRegister::IP].u = 0;
			registers[SpecialRegister::SP].u = 0;
			registers[SpecialRegister::FP].u = 0;
			registers[SpecialRegister::FL].u = 0;

		}

		void Raise(ErrorCode ecode) 
		{
			ErrorType etype = ErrorsMap::Get().GetType(ecode);
			ErrorFrame frame = ErrorFrame(registers[SpecialRegister::IP].u, registers[SpecialRegister::FP].u, registers[SpecialRegister::SP].u, ecode, etype);
			
			error_stack.push(frame);

			if (etype == ErrorType::CRITICAL)
			{
				registers[SpecialRegister::FL].u |= Flag::STOPPED;
			}
		}

	};

}