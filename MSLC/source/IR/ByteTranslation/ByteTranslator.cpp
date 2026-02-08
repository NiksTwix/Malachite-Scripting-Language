#include "..\..\..\include\IR\ByteTranslation\ByteTranslator.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			std::shared_ptr<ByteTranslationState>  ByteTranslator::Translate(Pseudo::POperationArray& p_array,CompilationInfo::CompilationState* c_state, ByteTranslationConfig config)
			{
				std::shared_ptr<ByteTranslationState> b_state = std::make_shared<ByteTranslationState>(config,c_state);
				b_state->cs_observer = c_state;
				for (; b_state->pseudo_ip < p_array.Size(); b_state->pseudo_ip++)
				{
					commands_handler.HandleCommand(p_array, b_state);
				}


				return b_state;
			}
		}
	}
}