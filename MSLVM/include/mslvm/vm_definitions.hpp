#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
namespace MSLVM
{
	using memory_cell = uint8_t;
	using register_index = uint64_t;

	using register_integer = int64_t;
	using register_unsigned = uint64_t;
	using register_real = double;
	using register_pointer = uint64_t;

	constexpr register_index TOTAL_REGISTERS = 128;
	constexpr register_index GENERAL_REGISTERS = 120;   // R0-R119
	constexpr register_index SPECIAL_REGISTERS = 8;     // R120-R127

	constexpr std::string_view msli_magic = "MSLI";
	constexpr float msli_version = 1.0f;
	constexpr uint8_t vm_version_code = 1;
	enum SpecialRegister : uint8_t {
		SP = 120,    // Stack Pointer (R120)
		FP = 121,    // Frame Pointer (R121)
		IP = 122,    // Instruction Pointer (R122)
		FL = 123,    // Flags (R123)
		A0 = 124,    // Argument/Accumulator 0 (R124)
		A1 = 125,    // Argument/Accumulator 1 (R125)
		A2 = 126,    // Argument/Accumulator 2 (R126)
		RT = 127     // Return Value Temporary (R127)
	};

	constexpr size_t MinStackSize = UINT16_MAX / 2;
	constexpr size_t MinHeapSize = UINT16_MAX / 2;

	constexpr size_t INVALID_ADDRESS = UINT64_MAX;

	constexpr size_t DEFAULT_ALIGNMENT = 8;

	constexpr uint32_t CALL_STACK_SIZE = 256;
	constexpr uint32_t ERROR_STACK_SIZE = 64;
	//In the future for dynamic linear memory
	struct VMConfig
	{
		size_t heap_size = 64 * 1024 * 1024;    // 64  MB default
		size_t stack_size = 1 * 1024 * 1024;    // 1 MB default
		size_t call_stack_size = 256;           // 256 frames

		// Можно настраивать
		VMConfig() = default;
		VMConfig(size_t heap, size_t stack, size_t calls = 256)
			: heap_size(heap), stack_size(stack), call_stack_size(calls) {
		}

	};


	enum Flag
	{
		ZERO = 1 << 0,		//EQUAL
		GREATER = 1 << 1,
		LESS = 1 << 2,
		JUMPED = 1 << 3,
		STOPPED = 1 << 4,
	};

	enum VMOperationCode : uint8_t
	{
		NOP = 0,

		// Arithmetic 
		ADD_RRR_REAL,
		SUB_RRR_REAL,
		DIV_RRR_REAL,
		MUL_RRR_REAL,
		NEG_RR_REAL,

		ADD_RRR_INTEGER,
		SUB_RRR_INTEGER,
		DIV_RRR_INTEGER,
		MUL_RRR_INTEGER,
		NEG_RR_INTEGER,
		MOD_RRR_INTEGER,

		ADD_RRR_UNSIGNED,
		SUB_RRR_UNSIGNED,
		DIV_RRR_UNSIGNED,
		MUL_RRR_UNSIGNED,
		MOD_RRR_UNSIGNED,

		//Logic

		AND_RRR,
		OR_RRR,
		NOT_RR,
		BIT_OR_RRR,
		BIT_NOT_RR,
		BIT_AND_RRR,
		BIT_OFFSET_LEFT_RRR,
		BIT_OFFSET_RIGHT_RRR,

		CMP_RR_INTEGER,          //arg0 - first, arg1 - second, arg2 - null
		CMP_RR_UNSIGNED,          //arg0 - first, arg1 - first, arg2 - null
		CMP_RR_REAL,     //arg0 - first, arg1 - second, arg2 - null; For double with nan checking
		//Memory

		//LOAD_RM,    //register-arg0,             address loading from - arg1, size [1-8 bytes] - arg2
		//STORE_MR,        //Address saving to-arg0,    register - arg1,             size [1-8 bytes] - arg2

		MOV_RR,
		MOV_RI,				//arg0 - register-destination, arg1 - immediated
		PUSH,                // arg0[register from], arg1[size (not register)]
		POP,                 // arg0[register to], arg1[size (not register)]

		//Current scope
		LOAD_ABSOLUTE,			//arg0[register]            arg1[immediate value of memory-offset]          arg2[size in bytes] // independed by FP
		LOAD_LOCAL,          //arg0[register]            arg1[immediate value of memory-offset]          arg2[size in bytes]  // depended by FP
		STORE_LOCAL,         //arg0[register]       arg1[immediate value of memory-offset]               arg2[size in bytes]   // depended by FP

		CALC_FRAME_ADDRESS,		//register(ARG0) offset(ARG1)
		LOAD_BY_ADDRESS,			// register(ARG0), register - address(ARG1), size(ARG2)
		STORE_BY_ADDRESS,			// register(ARG0), register - address(ARG1), size(ARG2)

		LOAD_CONST_LOCAL,		//register-dest, offset in rod, size
		LOAD_CONST_BY_ADDRESS,	//register-dest, reg with address, size

		ALLOCATE_MEMORY,		//arg0[register of address's saving], arg1[register with size of memory's interval]
		FREE_MEMORY,			//arg0[register with address], arg1[register with size of memory's interval]
		GRAB_FRAME,				//arg0[bytes] If bytes < 0 => grab to up, else - grab to down 
		// Control flow arg0 = where
		JMP,
		JMP_CV,      //CV- Condition Valid - arg0[where], arg1[condition register]
		JMP_CNV,     //CNV - Condition Not Valid - arg0[where], arg1[condition register]
		CALL,
		RET,
		HALT,	//EXT

		// Calls 
		VM_CALL,   //arg0[VMCall], arg1[parameter0], arg2[parameter1]


		// Type Convertion
		TC_ITR_R,    //Type Convertion Integer To Real
		TC_RTI_R,    //Type Convertion Real To Integer

		TC_UTR_R,    //Type Convertion Unsigned Integer To Real
		TC_UTI_R,    //Type Convertion Unsigned Integer To Integer

		TC_RTU_R,    //Type Convertion Real To Unsigned Integer
		TC_ITU_R,    //Type Convertion Integer To Unsigned Integer

	};


	enum VMCallType 
	{
		//PRINTING
		PRINT_INTEGER,		//arg0[VMCall], arg1[register_from]
		PRINT_REAL,			//arg0[VMCall], arg1[register_from]
		PRINT_UNSIGNED,		//arg0[VMCall], arg1[register_from]
		PRINT_CHAR,			//arg0[VMCall], arg1[register_from]
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
		VMOperation(VMOperationCode code, Register arg0, Register arg1, Register arg2) : code(code), arg0(arg0), arg1(arg1), arg2(arg2) {}
		VMOperation(VMOperationCode code, Register arg0, Register arg1) : code(code), arg0(arg0), arg1(arg1) {}
		VMOperation(VMOperationCode code, Register arg0) : code(code), arg0(arg0) {}
		VMOperation() {}
	};

}