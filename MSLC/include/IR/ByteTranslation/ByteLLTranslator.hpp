#pragma once
#include "ByteTranslationInfo.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			class LLTranslator 
			{
				std::unordered_map<Pseudo::LowLevelRegisters, uint8_t> allocated_registers;		//ID->Allocated register. 
				std::unordered_map<Pseudo::LowLevelOpCode, ByteOpCode> op_code_conversions =
				{
					{Pseudo::LowLevelOpCode::ADDI,ByteOpCode::ADDI},
					{Pseudo::LowLevelOpCode::SUBI,ByteOpCode::SUBI},
					{Pseudo::LowLevelOpCode::MULI,ByteOpCode::MULI},
					{Pseudo::LowLevelOpCode::DIVI,ByteOpCode::DIVI},
					{Pseudo::LowLevelOpCode::MODI,ByteOpCode::MODI},
					{Pseudo::LowLevelOpCode::NEGI,ByteOpCode::NEGI},
				};

				uint8_t GetAllocatedRegister(std::shared_ptr<ByteTranslationState>  state, Pseudo::LowLevelRegisters id);

				void HandleLowLevel(std::shared_ptr<ByteTranslationState>  state, const Pseudo::LLOperation& operation);

			public:

				void FreeAllocatedRegisters(std::shared_ptr<ByteTranslationState> state);

				void InsertLowLevel(std::shared_ptr<ByteTranslationState>  state);//PushLLOpers
			};
		}
	}
}