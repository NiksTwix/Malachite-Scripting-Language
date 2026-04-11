#pragma once
#include "ByteCommandsHandler.hpp"

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
				std::shared_ptr<ByteTranslationState> Translate(std::shared_ptr<Pseudo::PseudoTranslationState> pseudo_state, CompilationInfo::CompilationState* c_state, ByteTranslationConfig config);
			};
		}

	}
}