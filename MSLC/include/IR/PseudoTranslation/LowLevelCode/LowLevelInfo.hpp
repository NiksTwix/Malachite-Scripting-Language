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
				LEA,	//Register-dest, identificator Load address of variable to register
				DLEA,	//Register-dest, identificator Load straightaway in register pointer which contains in variable
				STORE,	//Register-dest,Register-src, size
				LOAD,	//Register-dest,Register-src[reg with address],size
				ADDI,	//Register-dest,register-src0,register-src1
				SUBI,	//Register-dest,register-src0,register-src1
				MULI,	//Register-dest,register-src0,register-src1
				DIVI,	//Register-dest,register-src0,register-src1
				MODI,	//Register-dest,register-src0,register-src1
				NEGI,	//Register-dest,register-src0
				MOVRI,	//Register-dest, immediate-src 

				COPY,	//Register-dest,register-src0,register-size

				MOV,	//Register-dest,register-src0
				SPEC_CALL,	//SPECIAL_CALL| call_id (depends by VM's version), arg0,arg1
				//PRINT_REG,	//(char/int/real), register-source
				LABEL,	//label id	-> ByteLowLevelTranslator will save it in labels table 
				JMP,	//label id
				JMP_IF, //label id, register
				JMP_NIF, //label id, register
				ALLOC,	//register for pointer, register with size in bytes, reset memory (1 or 0). If is third arg is null - automaticaly sets to 1
				FREE,	//register with pointer, register with size in bytes


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

			enum class LLArgumentSource 
			{
				Immediate = 0,
				RegisterID,
				VariableID,
				
			};


			struct LLArgument 
			{
				LLArgumentSource source = LLArgumentSource::Immediate;
				size_t data = 0;

				LLArgument() = default;
				LLArgument(size_t data) : source(LLArgumentSource::Immediate), data(data) {}
				LLArgument(LLArgumentSource src, size_t data) : source(src), data(data) {}
			};


			struct LLOperation
			{
				LowLevelOpCode code = LowLevelOpCode::NOP;
				LLArgument arg0 = 0;
				LLArgument arg1 = 0;
				LLArgument arg2 = 0;
				Diagnostics::DebugInfo debug_info;
			};


			class LLTranslationMap {
			
				std::unordered_map<std::string, LowLevelOpCode> string_codes =
				{
					{"NOP",NOP},
					{"LEA",LEA},
					{"DLEA",DLEA},
					{"STORE",STORE},
					{"LOAD",LOAD},
					{"ADDI",ADDI},
					{"SUBI",SUBI},
					{"MULI",MULI},
					{"DIVI",DIVI},
					{"MODI",MODI},
					{"NEGI",NEGI},
					{"MOV",MOV},
					{"MOVRI",MOVRI},
					{"SPEC_CALL",SPEC_CALL},
					{"LABEL",LABEL},
					{"JMP",JMP},
					{"JMP_IF",JMP_IF}, //label id, register
					{"JMP_NIF",JMP_NIF}, //label id, register
					{"ALLOC",ALLOC},
					{"FREE",FREE},
					{"COPY",COPY},
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