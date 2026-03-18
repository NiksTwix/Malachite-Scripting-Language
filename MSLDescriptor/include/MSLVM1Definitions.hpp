#pragma once
#include "Definitions.hpp"


namespace MSLVM1 
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

	constexpr uint8_t vm_index = 1;

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

	struct StringsMap 
	{
		std::unordered_map<VMOperationCode, std::string> vm_opcode_to_string = {
			// Арифметика (вещественная)
			{VMOperationCode::ADD_RRR_REAL, "ADD_RRR_REAL"},
			{VMOperationCode::SUB_RRR_REAL, "SUB_RRR_REAL"},
			{VMOperationCode::DIV_RRR_REAL, "DIV_RRR_REAL"},
			{VMOperationCode::MUL_RRR_REAL, "MUL_RRR_REAL"},
			{VMOperationCode::NEG_RR_REAL, "NEG_RR_REAL"},

			// Арифметика (целая, знаковая)
			{VMOperationCode::ADD_RRR_INTEGER, "ADD_RRR_INTEGER"},
			{VMOperationCode::SUB_RRR_INTEGER, "SUB_RRR_INTEGER"},
			{VMOperationCode::DIV_RRR_INTEGER, "DIV_RRR_INTEGER"},
			{VMOperationCode::MUL_RRR_INTEGER, "MUL_RRR_INTEGER"},
			{VMOperationCode::NEG_RR_INTEGER, "NEG_RR_INTEGER"},
			{VMOperationCode::MOD_RRR_INTEGER, "MOD_RRR_INTEGER"},

			// Арифметика (беззнаковая)
			{VMOperationCode::ADD_RRR_UNSIGNED, "ADD_RRR_UNSIGNED"},
			{VMOperationCode::SUB_RRR_UNSIGNED, "SUB_RRR_UNSIGNED"},
			{VMOperationCode::DIV_RRR_UNSIGNED, "DIV_RRR_UNSIGNED"},
			{VMOperationCode::MUL_RRR_UNSIGNED, "MUL_RRR_UNSIGNED"},
			{VMOperationCode::MOD_RRR_UNSIGNED, "MOD_RRR_UNSIGNED"},

			// Логические операции
			{VMOperationCode::AND_RRR, "AND_RRR"},
			{VMOperationCode::OR_RRR, "OR_RRR"},
			{VMOperationCode::NOT_RR, "NOT_RR"},

			// Битовые операции
			{VMOperationCode::BIT_OR_RRR, "BIT_OR_RRR"},
			{VMOperationCode::BIT_NOT_RR, "BIT_NOT_RR"},
			{VMOperationCode::BIT_AND_RRR, "BIT_AND_RRR"},
			{VMOperationCode::BIT_OFFSET_LEFT_RRR, "BIT_OFFSET_LEFT_RRR"},
			{VMOperationCode::BIT_OFFSET_RIGHT_RRR, "BIT_OFFSET_RIGHT_RRR"},

			// Сравнения
			{VMOperationCode::CMP_RR_INTEGER, "CMP_RR_INTEGER"},
			{VMOperationCode::CMP_RR_UNSIGNED, "CMP_RR_UNSIGNED"},
			{VMOperationCode::CMP_RR_REAL, "CMP_RR_REAL"},

			// Перемещения
			{VMOperationCode::MOV_RR, "MOV_RR"},
			{VMOperationCode::MOV_RI, "MOV_RI"},

			// Стек
			{VMOperationCode::PUSH, "PUSH"},
			{VMOperationCode::POP, "POP"},

			// Локальная память

			{VMOperationCode::LOAD_ABSOLUTE, "LOAD_ABSOLUTE"},
			{VMOperationCode::LOAD_LOCAL, "LOAD_LOCAL"},
			{VMOperationCode::STORE_LOCAL, "STORE_LOCAL"},

			// Адресная арифметика
			{VMOperationCode::CALC_FRAME_ADDRESS, "CALC_FRAME_ADDRESS"},
			{VMOperationCode::LOAD_BY_ADDRESS, "LOAD_BY_ADDRESS"},
			{VMOperationCode::STORE_BY_ADDRESS, "STORE_BY_ADDRESS"},

			// Константы
			{VMOperationCode::LOAD_CONST_LOCAL, "LOAD_CONST_LOCAL"},
			{VMOperationCode::LOAD_CONST_BY_ADDRESS, "LOAD_CONST_BY_ADDRESS"},

			// Управление памятью
			{VMOperationCode::ALLOCATE_MEMORY, "ALLOCATE_MEMORY"},
			{VMOperationCode::FREE_MEMORY, "FREE_MEMORY"},
			{VMOperationCode::GRAB_FRAME, "GRAB_FRAME"},

			// Управление потоком
			{VMOperationCode::JMP, "JMP"},
			{VMOperationCode::JMP_CV, "JMP_CV"},
			{VMOperationCode::JMP_CNV, "JMP_CNV"},
			{VMOperationCode::CALL, "CALL"},
			{VMOperationCode::RET, "RET"},
			{VMOperationCode::HALT, "HALT"},

			// Вызовы VM
			{VMOperationCode::VM_CALL, "VM_CALL"},

			// Преобразование типов
			{VMOperationCode::TC_ITR_R, "TC_ITR_R"},
			{VMOperationCode::TC_RTI_R, "TC_RTI_R"},
			{VMOperationCode::TC_UTR_R, "TC_UTR_R"},
			{VMOperationCode::TC_UTI_R, "TC_UTI_R"},
			{VMOperationCode::TC_RTU_R, "TC_RTU_R"},
			{VMOperationCode::TC_ITU_R, "TC_ITU_R"}
		};

		std::string opcode_to_string(VMOperationCode code) {
			auto it = vm_opcode_to_string.find(code);
			if (it != vm_opcode_to_string.end()) {
				return it->second;
			}
			return "UNKNOWN_OPCODE(" + std::to_string(static_cast<int>(code)) + ")";
		}
	};



	struct CodeChecker 
	{
		StringsMap cmap;
		

		bool IsBytesValid(size_t bytes_size) 
		{
			return bytes_size % sizeof(VMOperation) == 0;
		}

		size_t CalculateCodeSize(size_t bytes_size)
		{
			return bytes_size / sizeof(VMOperation);
		}
		
		void PrintIndex(char* pointer,size_t index) 
		{
			VMOperation* oper = reinterpret_cast<VMOperation*>(pointer) + index;

			std::cout << index << "|" << cmap.opcode_to_string(oper->code) << "|" << oper->arg0.u << "\t(" << oper->arg0.r << ";" << oper->arg0.i << ")" <<
				"|" << oper->arg1.u << "\t(" << oper->arg1.r << ";" << oper->arg1.i << ")" <<
				"|" << oper->arg2.u << "\t(" << oper->arg2.r << ";" << oper->arg2.i << ")" <<
				"\n";
		}
	};
}
