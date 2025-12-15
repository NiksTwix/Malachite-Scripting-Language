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
		memory_cell heap[HEAP_SIZE]{};
		memory_cell stack[STACK_SIZE]{};
		HeapFreeIntervals HFI{0, HEAP_END};
	};



	struct alignas(8) Register
	{
		union
		{
			register_integer i;
			register_unsigned u;	//unsigned int/pointer
			register_real r;
		};
		Register()
		{
			u = 0;
		}
		explicit Register(register_integer imm_i)
		{
			i = imm_i;
		}
		Register(register_unsigned imm_u)	//Register number
		{
			u = imm_u;
		}
		explicit Register(register_real imm_r)
		{
			r = imm_r;
		}
	};

	struct alignas(8) VMOperation	//32 bytes
	{
		Register arg0{};	//8 byte
		Register arg1{};	//8 byte
		Register arg2{};	//8 byte
		uint32_t dsi{};			//4 byte
		VMOperationCode code{};	//1 byte
		VMOperation(VMOperationCode code, Register arg0, Register arg1, Register arg2, uint32_t debug_symbol_index) : code(code), arg0(arg0), arg1(arg1), arg2(arg2), dsi(debug_symbol_index) {}
		VMOperation(VMOperationCode code, Register arg0, Register arg1, Register arg2) : code(code), arg0(arg0), arg1(arg1), arg2(arg2){}
		VMOperation(VMOperationCode code, Register arg0, Register arg1) : code(code), arg0(arg0), arg1(arg1){}
		VMOperation(VMOperationCode code, Register arg0) : code(code), arg0(arg0) {}
		VMOperation() {}
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