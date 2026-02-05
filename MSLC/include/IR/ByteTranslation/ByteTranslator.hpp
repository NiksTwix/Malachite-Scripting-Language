#pragma once
#include "ByteTranslationInfo.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			
			class ByteTranslator 
			{
				void HandleCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
			public:
				std::shared_ptr<ByteTranslationState> Translate(Pseudo::POperationArray& p_array, CompilationInfo::CompilationState* c_state, ByteTranslationConfig config);
			};
		}

	}
}