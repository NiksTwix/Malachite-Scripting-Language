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

				ByteOpCode GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode code, ValueFrame::ValueNativeType type);
				ByteCommand GetConversionCommand(ValueFrame::ValueNativeType from, ValueFrame::ValueNativeType to, CommandArgument reg);
				ByteCommand GetTypeConvertionCommand(ValueFrame::ValueNativeType first, CommandArgument first_register, ValueFrame::ValueNativeType second, CommandArgument second_register, CommandArgument& converted_register, ValueFrame::ValueNativeType& result_type);
			public:
				void HandleCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
			};
			

		}
	}
}