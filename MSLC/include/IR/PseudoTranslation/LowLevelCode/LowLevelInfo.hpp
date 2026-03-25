#pragma once
#include "..\..\..\Definitions\ValueContainer.hpp"



namespace MSLC {
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{

			enum LowLevelOpCode : uint8_t
			{
				NOP,
				LEA,	//Register-dest, identificator Load address of variable to register (works only with static pointers (offsets))
				DLEA,	//Register-dest, identificator Load in register dynamic pointer
				STORE,	// Register-dest,Register-src, size
				LOAD,	// Register-dest,Register-src	size
				ADD,	//Register-dest,register-src0,register-src1
				SUB,	//Register-dest,register-src0,register-src1
				MUL,	//Register-dest,register-src0,register-src1
				DIV,	//Register-dest,register-src0,register-src1
				MOD,	//Register-dest,register-src0,register-src1
				NEG,	//Register-dest,register-src0
				MOVE,	//Register-dest,register-src0
				PRINT,	//print_type(int,char,real, and them ptr versions), register-source, size(for ptrs and strings)
				LABEL,	//label id	-> ByteLowLevelTranslator will save it in labels table 
				JUMP,	//label id
			};


			struct LLOperation
			{
				LowLevelOpCode code = LowLevelOpCode::NOP;
				size_t arg0 = 0;
				size_t arg1 = 0;
				size_t arg2 = 0;
				size_t source_line = 0;
			};


			class LLTranslationMap {
			
				std::unordered_map<std::string, LowLevelOpCode> string_codes =
				{
					{"NOP",NOP},
					{"LEA",LEA },
					{"DLEA",DLEA},
					{"STORE",STORE},
					{"LOAD",LOAD},
					{"ADD",ADD},
					{"SUB",SUB},
					{"MUL",MUL},
					{"DIV",DIV},
					{"MOD",MOD},
					{"NEG",NEG},
					{"MOVE",MOVE},
					{"PRINT",PRINT},
				};
				std::unordered_map<std::string, uint8_t> registers_id =
				{
					{"RA",1},
					{"RB",2},
					{"RC",3},
					{"RD",4},
					{"RE",5},
					{"RF",6},
					{"RG",7},
					{"RH",8},
				};
			public:

				LowLevelOpCode GetCode(const std::string& literal) 
				{
					auto it = string_codes.find(literal);
					if (it == string_codes.end()) return LowLevelOpCode::NOP;
					return it->second;
				}
				uint8_t GetRegisterID(const std::string& literal)
				{
					auto it = registers_id.find(literal);
					if (it == registers_id.end()) return 0;
					return it->second;
				}
				static LLTranslationMap& Get() 
				{
					static LLTranslationMap map;
					return map;
				}
			};



		}
	}
}