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
		GET_FLAG,		//arg0 - dest, arg1 - FLAG_NUMBER0 , arg1 - FLAG_NUMBER1 In variable flag mode, the OR operation is used.
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
		STATIC_COPY,			//arg0[immediate with address-dest],arg0[immediate with address-dest],arg0[size]
		DYNAMIC_COPY,			//arg0[register with pointer-dest],arg0[register with pointer-src],arg0[register with size]
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
		//DIAGNOSTIC
		GET_ERROR,			//arg0[VMCall], arg1[register_to]
	};
	enum Flag : uint64_t
	{
		NONE = 0,
		ZERO = 1 << 0,		//EQUAL
		GREATER = 1 << 1,
		LESS = 1 << 2,
		JUMPED = 1 << 3,
		STOPPED = 1 << 4,
	};
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
			{VMOperationCode::GET_FLAG, "GET_FLAG"},
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
		std::unordered_map<VMCallType, std::string> vm_calls = {
			{VMCallType::PRINT_CHAR, "PRINT_CHAR"},
			{VMCallType::PRINT_INTEGER, "PRINT_INTEGER"},
			{VMCallType::PRINT_REAL, "PRINT_REAL"},
			{VMCallType::PRINT_UNSIGNED, "PRINT_UNSIGNED"},
			{VMCallType::GET_ERROR, "GET_ERROR"},
		};

		std::unordered_map<Flag, std::string> vm_flags = {
			{Flag::NONE, "NONE"},
			{Flag::GREATER, "GREATER"},
			{Flag::JUMPED, "JUMPED"},
			{Flag::LESS, "LESS"},
			{Flag::ZERO, "ZERO"},
			{Flag::STOPPED, "STOPPED"},
		};
		std::unordered_map<SpecialRegister, std::string> vm_sregs = {
			{SP , "Stack pointer"},
			{FP	, "Frame pointer"},
			{IP	, "Instruction pointer"},
			{FL , "Flags"},
			{A0 , "Accumulator 0"},
			{A1	, "Accumulator 1"},
			{A2	, "Accumulator 2"},
			{RT	, "Returned temporary value"},
		};



		std::string opcode_to_string(VMOperationCode code) {
			auto it = vm_opcode_to_string.find(code);
			if (it != vm_opcode_to_string.end()) {
				return it->second;
			}
			if (code == VMOperationCode::NOP) return "NOP";
			return "UNKNOWN_OPCODE(" + std::to_string(static_cast<int>(code)) + ")";
		}
		std::string flag_to_string(Flag flag) {
			auto it = vm_flags.find(flag);
			if (it != vm_flags.end()) {
				return it->second;
			}
			return "UNKNOWN_FLAG(" + std::to_string(static_cast<int>(flag)) + ")";
		}
		std::string vmcall_to_string(VMCallType type) {
			auto it = vm_calls.find(type);
			if (it != vm_calls.end()) {
				return it->second;
			}
			return "UNKNOWN_VMCALL(" + std::to_string(static_cast<int>(type)) + ")";
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

			std::cout << index << "|" << cmap.opcode_to_string(oper->code) << "|";

			if (oper->code == VMOperationCode::VM_CALL) 
			{
				std::cout << cmap.vmcall_to_string((VMCallType)oper->arg0.u) <<
					"|" << oper->arg1.u << "\t(" << oper->arg1.r << ";" << oper->arg1.i << ")" <<
					"|" << oper->arg2.u << "\t(" << oper->arg2.r << ";" << oper->arg2.i << ")" <<
					"\n";
			}
			else 
			{
				std::cout << oper->arg0.u << "\t(" << oper->arg0.r << ";" << oper->arg0.i << ")" <<
					"|" << oper->arg1.u << "\t(" << oper->arg1.r << ";" << oper->arg1.i << ")" <<
					"|" << oper->arg2.u << "\t(" << oper->arg2.r << ";" << oper->arg2.i << ")" <<
					"\n";
			}
		}
		void PrintVMInfo()
		{
			std::cout << "MSLVM_1 info.\n";

			std::cout << "VM version: "<< (int)vm_index <<".\n";
			std::cout << "Count of general registers: " << GENERAL_REGISTERS << ".\n";
			std::cout << "Count of special registers: " << SPECIAL_REGISTERS << ".\n";
			std::cout << "Special registers:\n";
			for (auto p : cmap.vm_sregs)
			{
				std::cout << "\t" << p.second << " (" << (int)p.first << ").\n";
			}
			std::cout << "VMCalls:\n";
			for (auto p : cmap.vm_calls) 
			{
				std::cout << "\t" << p.second << " (" << (int)p.first << ").\n";
			}
			std::cout << "VMFlags:\n";
			for (auto p : cmap.vm_flags)
			{
				std::cout << "\t" << p.second << " (" << (int)p.first << ").\n";
			}
		}
	};
}
