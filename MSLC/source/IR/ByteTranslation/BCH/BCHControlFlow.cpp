#include "..\..\..\..\include\IR\ByteTranslation\BCH\BCHControlFlow.hpp"


namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			namespace BCH
			{
				void ControlFlowHandler::Handle(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state) 
				{
					auto operation = p_array[b_state->pseudo_ip];

					auto push_command = [](std::shared_ptr<ByteTranslationState> state, ByteCommand cmd)
						{
							cmd.source_line = state->declaring_place.place;
							cmd.pseudo_op_index = state->result.Size();
							state->result.Pushback(cmd);
						};

					switch (operation.op_code)
					{
						case Pseudo::PseudoOpCode::Label:
						{
							ByteCommand command;
							command.code = ByteOpCode::JMP_LABEL;
							command.arg0 = CommandArgument(operation.arg_0, CommandSource::Immediate); //Label id
							command.source_line = b_state->declaring_place.place;
							push_command(b_state,command);
						}
							
							break;
						case  Pseudo::PseudoOpCode::Jump:
						{
							ByteCommand command;
							command.code = ByteOpCode::JMP;
							command.arg0 = CommandArgument(operation.arg_0, CommandSource::Immediate); //Label id
							push_command(b_state, command);
						}
							break;
						case Pseudo::PseudoOpCode::JumpNIf:
						case Pseudo::PseudoOpCode::JumpIf:
						{
							ByteCommand command;
							command.code = operation.op_code == Pseudo::PseudoOpCode::JumpNIf ? ByteOpCode::JMPCNV: ByteOpCode::JMPCV;

							ValueFrame condition_source = GenerateLoadCommand(p_array, b_state);
							command.arg0 = CommandArgument(operation.arg_0, CommandSource::Immediate); //Label id
							command.arg1 = CommandArgument(condition_source.data, CommandSource::Register); //Condition's source

							push_command(b_state, command);
						}
							break;
					}
				
				}
			}
		}
	}
}
