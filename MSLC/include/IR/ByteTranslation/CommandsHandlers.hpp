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

				ValueFrame GenerateLoadCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);	//returns new source of value


				ByteOpCode GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode code, PrimitiveAnalogs type);
				ByteCommand GetConversionCommand(PrimitiveAnalogs from, PrimitiveAnalogs to, CommandArgument reg);
				ByteCommand GetTypeConvertionCommand(PrimitiveAnalogs first, CommandArgument first_register, PrimitiveAnalogs second, CommandArgument second_register, CommandArgument& converted_register, PrimitiveAnalogs& result_type);
				PrimitiveAnalogs ValueContainerTypeToPrimitive(Definitions::ValueType type);

				void PushCommand(std::shared_ptr<ByteTranslationState> b_state, ByteCommand&& command, size_t line);

				bool CheckRegister(std::shared_ptr<ByteTranslationState> b_state, size_t register_);

			public:
				void HandleCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
			};
			

		}
	}
}