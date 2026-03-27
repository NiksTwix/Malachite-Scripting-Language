#include "..\..\..\include\IR\ByteTranslation\ByteTranslator.hpp"
#include "..\..\..\include\IR\ByteTranslation\ByteDebugInfo.hpp"
namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			std::shared_ptr<ByteTranslationState>  ByteTranslator::Translate(std::shared_ptr<Pseudo::PseudoTranslationState> pseudo_state,CompilationInfo::CompilationState* c_state, ByteTranslationConfig config)
			{
				std::shared_ptr<ByteTranslationState> b_state = std::make_shared<ByteTranslationState>(config,c_state);
				b_state->cs_observer = c_state;
				b_state->pseudo_state_observer = pseudo_state;
				for (; b_state->pseudo_ip < pseudo_state->pseudo_code.Size(); b_state->pseudo_ip++)
				{
					commands_handler.HandleCommand(b_state);
				}

				commands_handler.FreeTempResources(b_state);

				CommandStringSerializer css;
				std::cout << "Size: " << b_state->result.Size() << "\n";
				for (size_t i = 0; i < b_state->result.Size(); i++) 
				{
					std::cout << css.Serialize(b_state->result[i]) << "\n";
				}

				return b_state;
			}
		}
	}
}