#pragma once
#include "vm_definitions.hpp"
#include "vm_errors.hpp"
#include <vector>
#include "vm_memory.hpp"

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
		explicit Register(register_unsigned imm_u)
		{
			u = imm_u;
		}
		explicit Register(register_real imm_r)
		{
			r = imm_r;
		}
	};

	struct alignas(8) VMOperation	//16 bytes
	{
		Register imm{};			//8 bytes
		uint32_t dsi{};			//debug symbol index 2 bytes
		VMOperationCode code{};	//1 byte
		register_index dest{};	//1 byte
		register_index src0{};	//1 byte
		register_index src1{};	//1 byte

		VMOperation(VMOperationCode code, register_index dest, register_index src0, register_index src1, Register imm, uint32_t debug_symbol_index) : code(code), dest(dest), src0(src0), src1(src1), imm(imm), dsi(debug_symbol_index) {}
		VMOperation(VMOperationCode code, register_index dest, register_index src0, register_index src1, Register imm) : code(code), dest(dest), src0(src0), src1(src1), imm(imm) {}
		VMOperation(VMOperationCode code, register_index dest, register_index src0, register_index src1) : code(code), dest(dest), src0(src0), src1(src1) {}
		VMOperation(VMOperationCode code, register_index dest, register_index src0) : code(code), dest(dest), src0(src0){}
		VMOperation(VMOperationCode code, register_index dest, Register imm) : code(code), dest(dest), imm(imm) {}
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