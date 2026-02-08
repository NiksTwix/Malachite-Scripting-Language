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
				void HandleArithmetic(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);

				void HandleMemory(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
				void HandleAssignment(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
				void HandleDeclaring(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);

				void AppendLoadCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);

				ByteOpCode GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode code, PrimitiveAnalogs type);
				ByteCommand GetConversionCommand(PrimitiveAnalogs from, PrimitiveAnalogs to, CommandArgument reg);
				ByteCommand GetTypeConvertionCommand(PrimitiveAnalogs first, CommandArgument first_register, PrimitiveAnalogs second, CommandArgument second_register, CommandArgument& converted_register, PrimitiveAnalogs& result_type);
				PrimitiveAnalogs ValueContainerTypeToPrimitive(Definitions::ValueType type);

				void PushCommand(std::shared_ptr<ByteTranslationState> b_state, ByteCommand&& command, size_t line);

			public:
				void HandleCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
			};
			

		}
	}
}