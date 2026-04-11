#include "..\..\..\include\IR\ByteTranslation\ByteCommandsHandler.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{


			void CommandsHandler::HandleCommand(std::shared_ptr<ByteTranslationState> b_state)
			{
				Pseudo::PseudoOperation& operation = b_state->pseudo_state_observer->pseudo_code.Get(b_state->pseudo_ip);
				b_state->current_line = b_state->current_line;
				if (operation.op_code == Pseudo::PseudoOpCode::PushFrame)
				{
					b_state->PushFrame(); 
					return;
				}
				if (operation.op_code == Pseudo::PseudoOpCode::PopFrame)
				{
					if (!b_state->PopFrame())
					{
						Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage("Extra visible's scope deleting. Operation line: %s.", Diagnostics::DeveloperError, Diagnostics::IRCode, b_state->pseudo_ip), b_state->current_line);
					}
					return;
				}
				if (operation.op_code == Pseudo::PseudoOpCode::PushLLOpers) 
				{
					ll_translator.InsertLowLevel(b_state);
				}

				//Standart handling

				if (operation.op_code > Pseudo::PseudoOpCode::ST_AL && operation.op_code < Pseudo::PseudoOpCode::ED_AL) 
				{
					alh.Handle(b_state->pseudo_state_observer->pseudo_code, b_state);
				}
				else if (operation.op_code > Pseudo::PseudoOpCode::ST_MEM && operation.op_code < Pseudo::PseudoOpCode::ED_MEM)
				{
					dfh.HandleMemory(b_state->pseudo_state_observer->pseudo_code, b_state, alh);
				}
				else if (operation.op_code > Pseudo::PseudoOpCode::ST_USING && operation.op_code < Pseudo::PseudoOpCode::ED_USING) 
				{
					dfh.HandleUsing(b_state->pseudo_state_observer->pseudo_code, b_state);
				}
				else if (operation.op_code > Pseudo::PseudoOpCode::ST_DECL && operation.op_code < Pseudo::PseudoOpCode::ED_DECL)
				{
					dfh.HandleDeclaring(b_state->pseudo_state_observer->pseudo_code, b_state);
				}
			}
			void CommandsHandler::FreeTempResources(std::shared_ptr<ByteTranslationState> b_state)
			{
				ll_translator.FreeAllocatedRegisters(b_state);
			}
		}
	}
}