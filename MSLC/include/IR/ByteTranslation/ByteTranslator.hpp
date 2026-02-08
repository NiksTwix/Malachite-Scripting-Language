#pragma once
#include "CommandsHandlers.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			
			class ByteTranslator 
			{
				CommandsHandler commands_handler;

				

			public:
				std::shared_ptr<ByteTranslationState> Translate(Pseudo::POperationArray& p_array, CompilationInfo::CompilationState* c_state, ByteTranslationConfig config);
			};
		}

	}
}