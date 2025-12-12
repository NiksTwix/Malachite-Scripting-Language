#pragma once
#include <cstdint>

namespace MSLVM
{
	using memory_cell = uint8_t;
	using register_index = uint8_t;

	using register_integer = int64_t;
	using register_unsigned = uint64_t;
	using register_real = double;
	using register_pointer = uint64_t;

	constexpr register_index TOTAL_REGISTERS = 128;
	constexpr register_index GENERAL_REGISTERS = 120;   // R0-R119
	constexpr register_index SPECIAL_REGISTERS = 8;     // R120-R127

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
	

	constexpr uint32_t HEAP_SIZE = 32768; //32 KB
	constexpr uint32_t HEAP_START = 0;
	constexpr uint32_t HEAP_END = HEAP_SIZE-1;

	constexpr uint32_t STACK_SIZE = 32768; //32 KB
	constexpr uint32_t STACK_START = 0;
	constexpr uint32_t STACK_END = STACK_SIZE - 1;

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

	using operation_code = uint16_t;

	enum VMOperationCode: uint8_t
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
		MOD_RR_INTEGER,

		ADD_RRR_UNSIGNED,
		SUB_RRR_UNSIGNED,
		DIV_RRR_UNSIGNED,
		MUL_RRR_UNSIGNED,
		MOD_RR_UNSIGNED,

		//Logic

		AND_RRR,
		OR_RRR,
		NOT_RR,
		BIT_OR_RRR,
		BIT_NOT_RR,
		BIT_AND_RRR,
		BIT_OFFSET_LEFT_RRR,
		BIT_OFFSET_RIGHT_RRR,

		CMP_RR_INTEGER,          //destination - null, source0 - first, source1 - second
		CMP_RR_UNSIGNED,          //destination - null, source0 - first, source1 - second
		CMP_RR_REAL,     //destination - null, source0 - first, source1 - second; For double with nan checking
		GET_FLAG,    //destination - register, source0 - flag type (check FLAG enum)

		//Memory

		//LOAD_RM,    //register-destination,             address loading from - source0, size [1-8 bytes] - source1
		//STORE_MR,        //Address saving to-destination,    register - source0,             size [1-8 bytes] - source1

		MOV_RR,
		MOV_RI_INTEGER,          //Integer
		MOV_RI_UNSIGNED,         //Unsigned integer
		MOV_RI_REAL,       //Double

		PUSH,                // destination[size (not register)], source0[register from]
		POP,                 // destination[register to], source0[size (not register)]

		//Current scope
		LOAD_LOCAL,          //destination[register]            source[memory-offset]          source1[size in bytes]  
		STORE_LOCAL,         //destination[memory-offset]       source[register]               source1[size in bytes]  
		//Global scope
		LOAD_GLOBAL,          //destination[register]            source[memory-offset]          source1[size in bytes]  
		STORE_GLOBAL,         //destination[memory-offset]       source[register]               source1[size in bytes]  

		// A - absolute, r - relatively
		//STORE_ENCLOSING_A,     //destination[memory-offset]       source0[register]              source1[size and depth] size - 32 little bits, depth - 32 big bits       we store variable to n frame at start
		//LOAD_ENCLOSING_A,      //destination[register]            source0[memory-offset]         source1[size and depth] size - 32 little bits, depth - 32 big bits       we load variable from n frame at start
		//STORE_ENCLOSING_R,     //destination[memory-offset]       source0[register]              source1[size and depth] size - 32 little bits, depth - 32 big bits       we store variable to n frame from top
		//LOAD_ENCLOSING_R,      //destination[register]            source0[memory-offset]         source1[size and depth] size - 32 little bits, depth - 32 big bits       we load variable from n frame  from top

		ALLOCATE_MEMORY,
		FREE_MEMORY,

		// Control flow destination = where
		JMP,
		JMP_CV,      //CV- Condition Valid - destination[where], source0[condition register]
		JMP_CNV,     //CNV - Condition Not Valid - destination[where], source0[condition register]
		CALL,
		RET,
		HALT,	//EXT

		// Calls 
		VM_CALL,   //destination[VMCall], source0[param0], source1[param1]


		// Other
		TC_ITD_R,    //Type Convertion Integer To Double
		TC_DTI_R,    //Type Convertion Double To Integer

		TC_UTD_R,    //Type Convertion Unsigned Integer To Double
		TC_UTI_R,    //Type Convertion Unsigned Integer To Integer
		TC_DTU_R,    //Type Convertion Double To Unsigned Integer
		TC_ITU_R,    //Type Convertion Integer To Unsigned Integer
	};		   


}