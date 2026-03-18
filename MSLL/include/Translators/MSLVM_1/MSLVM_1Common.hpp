#pragma once
#include "unordered_map"
#include "..\..\ObjectsReading\LinkDefinitions.hpp"
#include "..\..\ObjectsReading\ObjectsReader.hpp"
namespace MSLL
{
	namespace MSLVM_1
	{
		using memory_cell = uint8_t;
		using register_index = uint64_t;

		using register_integer = int64_t;
		using register_unsigned = uint64_t;
		using register_real = double;
		using register_pointer = uint64_t;


		constexpr size_t DEFAULT_ALIGNMENT = 8;

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
			LOAD_ABSOLUTE,			//arg0[register]            arg1[immediate value of memory-offset]          arg2[size in bytes] 
			LOAD_LOCAL,          //arg0[register]            arg1[immediate value of memory-offset]          arg2[size in bytes]  
			STORE_LOCAL,         //arg0[register]       arg1[immediate value of memory-offset]               arg2[size in bytes]  

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
		class OpCodeTable
		{
			std::unordered_map<LinkDefinitions::ByteOpCode, VMOperationCode> table =
			{
				{LinkDefinitions::ByteOpCode::ADDR,VMOperationCode::ADD_RRR_REAL	},
				{LinkDefinitions::ByteOpCode::SUBR,VMOperationCode::SUB_RRR_REAL	},
				{LinkDefinitions::ByteOpCode::DIVR,VMOperationCode::DIV_RRR_REAL	},
				{LinkDefinitions::ByteOpCode::MULR,VMOperationCode::MUL_RRR_REAL	},
				{LinkDefinitions::ByteOpCode::NEGR,VMOperationCode::NEG_RR_REAL		},
				{LinkDefinitions::ByteOpCode::EXPR,VMOperationCode::NOP				},//Check!
				{LinkDefinitions::ByteOpCode::ADDI,VMOperationCode::ADD_RRR_INTEGER	},
				{LinkDefinitions::ByteOpCode::SUBI,VMOperationCode::SUB_RRR_INTEGER	},
				{LinkDefinitions::ByteOpCode::DIVI,VMOperationCode::DIV_RRR_INTEGER	},
				{LinkDefinitions::ByteOpCode::MULI,VMOperationCode::MUL_RRR_INTEGER	},
				{LinkDefinitions::ByteOpCode::NEGI,VMOperationCode::NEG_RR_INTEGER	},
				{LinkDefinitions::ByteOpCode::MODI,VMOperationCode::MOD_RRR_INTEGER	},
				{LinkDefinitions::ByteOpCode::EXPI,VMOperationCode::NOP				},//Check!
				{LinkDefinitions::ByteOpCode::ADDU,VMOperationCode::ADD_RRR_UNSIGNED},
				{LinkDefinitions::ByteOpCode::SUBU,VMOperationCode::SUB_RRR_UNSIGNED},
				{LinkDefinitions::ByteOpCode::DIVU,VMOperationCode::DIV_RRR_UNSIGNED},
				{LinkDefinitions::ByteOpCode::MULU,VMOperationCode::MUL_RRR_UNSIGNED},
				{LinkDefinitions::ByteOpCode::MODU,VMOperationCode::MOD_RRR_UNSIGNED},
				{LinkDefinitions::ByteOpCode::EXPU,VMOperationCode::NOP				},//Check!

				{LinkDefinitions::ByteOpCode::AND,				VMOperationCode::AND_RRR},
				{LinkDefinitions::ByteOpCode::OR,				VMOperationCode::OR_RRR},
				{LinkDefinitions::ByteOpCode::NOT,				VMOperationCode::NOT_RR},
				{LinkDefinitions::ByteOpCode::BIT_OR,			VMOperationCode::BIT_OR_RRR},
				{LinkDefinitions::ByteOpCode::BIT_NOT,			VMOperationCode::BIT_NOT_RR},
				{LinkDefinitions::ByteOpCode::BIT_AND,			VMOperationCode::BIT_AND_RRR},
				{LinkDefinitions::ByteOpCode::BIT_OFFSET_LEFT,	VMOperationCode::BIT_OFFSET_LEFT_RRR},
				{LinkDefinitions::ByteOpCode::BIT_OFFSET_RIGHT,	VMOperationCode::BIT_OFFSET_RIGHT_RRR},

				{LinkDefinitions::ByteOpCode::CMPI,VMOperationCode::CMP_RR_INTEGER},
				{LinkDefinitions::ByteOpCode::CMPU,VMOperationCode::CMP_RR_UNSIGNED},
				{LinkDefinitions::ByteOpCode::CMPR,VMOperationCode::CMP_RR_REAL}
			};
		public:
			static OpCodeTable& Get()
			{
				static OpCodeTable table;
				return table;
			}

			VMOperationCode At(LinkDefinitions::ByteOpCode boc)
			{
				auto it = table.find(boc);
				if (it == table.end()) return VMOperationCode::NOP;
				return it->second;
			}

		};
		
	}

}