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
				STORE,	//Register-dest,Register-src, size
				LOAD,	//Register-dest,Register-src,size
				ADDI,	//Register-dest,register-src0,register-src1
				SUBI,	//Register-dest,register-src0,register-src1
				MULI,	//Register-dest,register-src0,register-src1
				DIVI,	//Register-dest,register-src0,register-src1
				MODI,	//Register-dest,register-src0,register-src1
				NEGI,	//Register-dest,register-src0
				MOVE,	//Register-dest,register-src0
				//PRINT_REG,	//(char/int/real), register-source
				LABEL,	//label id	-> ByteLowLevelTranslator will save it in labels table 
				JUMP,	//label id
			};

			enum LowLevelRegisters : uint8_t 
			{
				InvalidR = 0xff,
				RA = 1,
				RB,
				RC,
				RD,
				RE,
				RF,
				RG,
				RH,
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
					{"ADDI",ADDI},
					{"SUBI",SUBI},
					{"MULI",MULI},
					{"DIVI",DIVI},
					{"MODI",MODI},
					{"NEGI",NEGI},
					{"MOVE",MOVE},
				};
				std::unordered_map<std::string, LowLevelRegisters> registers_id =
				{
					{"RA",RA},
					{"RB",RB},
					{"RC",RC},
					{"RD",RD},
					{"RE",RE},
					{"RF",RF},
					{"RG",RG},
					{"RH",RH},
				};
			public:

				LowLevelOpCode GetCode(const std::string& literal) 
				{
					auto it = string_codes.find(literal);
					if (it == string_codes.end()) return LowLevelOpCode::NOP;
					return it->second;
				}
				LowLevelRegisters GetRegisterID(const std::string& literal)
				{
					auto it = registers_id.find(literal);
					if (it == registers_id.end()) return LowLevelRegisters::InvalidR;
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