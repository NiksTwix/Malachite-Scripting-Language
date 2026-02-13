#pragma once
#include "ByteTranslationInfo.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			class CommandsHandler
			{
				void HandleMemory(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);

				void HandleUsing(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);

				ValueFrame GenerateLoadCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);	//returns new source of value

				void HandleAL(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);

				ByteOpCode GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode code, PrimitiveAnalogs type);
				ByteCommand GetConversionCommand(PrimitiveAnalogs from, PrimitiveAnalogs to, size_t reg);
				ByteCommand GetTypeConvertionCommand(PrimitiveAnalogs first, size_t first_register, PrimitiveAnalogs second, size_t second_register, size_t& converted_register, PrimitiveAnalogs& result_type);
				PrimitiveAnalogs ValueContainerTypeToPrimitive(Definitions::ValueType type);

				ByteOpCode GetLogicCommand(Pseudo::PseudoOpCode code, PrimitiveAnalogs type);


				void PushCommand(std::shared_ptr<ByteTranslationState> b_state, ByteCommand&& command, size_t line);
				bool CheckRegister(std::shared_ptr<ByteTranslationState> b_state, size_t register_);

			public:
				void HandleCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
			};
			

		}
	}
}