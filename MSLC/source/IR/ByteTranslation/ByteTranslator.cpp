#include "..\..\..\include\IR\ByteTranslation\ByteTranslator.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			void ByteTranslator::HandleCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
			{
				Pseudo::PseudoOperation& operation = p_array.Get(b_state->pseudo_ip);

				if (operation.op_code == Pseudo::PseudoOpCode::PushFrame) 
				{
					b_state->PushFrame(); return;
				}
				if (operation.op_code == Pseudo::PseudoOpCode::PopFrame)
				{
					if (!b_state->PopFrame()) 
					{	
						Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage("Extra visible's scope deleting. Operation line: %s.", Diagnostics::DeveloperError, Diagnostics::IRCode, b_state->pseudo_ip), operation.debug_line);
					}
					return;
				}

				//Standart handling
			}


			std::shared_ptr<ByteTranslationState>  ByteTranslator::Translate(Pseudo::POperationArray& p_array,CompilationInfo::CompilationState* c_state, ByteTranslationConfig config)
			{
				std::shared_ptr<ByteTranslationState> b_state = std::make_shared<ByteTranslationState>(config);
				b_state->cs_observer = c_state;
				for (; b_state->pseudo_ip < p_array.Size(); b_state->pseudo_ip++)
				{
					HandleCommand(p_array, b_state);
				}


				return b_state;
			}
		}
	}
}