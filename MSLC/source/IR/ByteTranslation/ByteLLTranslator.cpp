#include "..\..\..\include\IR\ByteTranslation\ByteLLTranslator.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			uint8_t LLTranslator::GetAllocatedRegister(std::shared_ptr<ByteTranslationState> state, Pseudo::LowLevelRegisters id)
			{
				auto it = allocated_registers.find(id);
				if (it != allocated_registers.end()) return it->second;
				
				size_t free_register = state->registers_table.AllocateFreeGeneral();
				if (free_register == InvalidRegister) 
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("The low level " + std::string(Keywords::w_op_code) + " section free registers has ran out.", Diagnostics::DeveloperError, Diagnostics::SourceCode, 0));
					return 0;
				
				}
				allocated_registers[id] = free_register;
				return free_register;
			}
			void LLTranslator::HandleLowLevel(std::shared_ptr<ByteTranslationState> state, const Pseudo::LLOperation& operation)
			{
				//operations handling

				auto push_command = [](std::shared_ptr<ByteTranslationState> state, uint32_t line, ByteCommand cmd)
					{
						cmd.source_line = line;
						cmd.pseudo_op_index = state->result.Size();
						state->result.Pushback(cmd);
					};



				switch (operation.code)
				{
				case Pseudo::LowLevelOpCode::NOP:
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("The " + std::string(Keywords::w_op_code) + " section's traslator has generated NOP operation. Is it well?", Diagnostics::DeveloperError, Diagnostics::SourceCode, operation.source_line));
					return;
				}
				case Pseudo::LowLevelOpCode::LEA:
				{
					uint8_t reg = GetAllocatedRegister(state,(Pseudo::LowLevelRegisters)operation.arg0.data);
					
					size_t variable_id = operation.arg1.data;

					if (state->dvicm[variable_id]) 
					{
						push_command(state, operation.source_line, ByteCommand(ByteOpCode::MOVRI, CommandArgument(reg, CommandSource::Register), CommandArgument(state->cs_observer->GetGST().GetVariable(variable_id)->global_stack_offset, CommandSource::MemoryAddress)));
					}
					else 
					{
						push_command(state, operation.source_line, ByteCommand(ByteOpCode::MOVRI, CommandArgument(reg, CommandSource::Register), CommandArgument(state->cs_observer->AddUnhandledSymbol(CompilationInfo::SymbolType::Variable, variable_id), CommandSource::Symbol)));
					}
				}
					break;
				case Pseudo::LowLevelOpCode::DLEA:
				{
					uint8_t reg = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg0.data);

					size_t variable_id = operation.arg1.data;
					if (state->dvicm[variable_id])
					{
						push_command(state, operation.source_line, 
							ByteCommand(ByteOpCode::LOAD_STATIC, CommandArgument(state->cs_observer->GetGST().GetVariable(variable_id)->global_stack_offset, CommandSource::MemoryAddress), CommandArgument(reg, CommandSource::Register), CommandArgument(POINTER_SIZE, CommandSource::Immediate)));
					}
					else
					{
						push_command(state, operation.source_line, 
							ByteCommand(ByteOpCode::LOAD_STATIC, CommandArgument(state->cs_observer->AddUnhandledSymbol(CompilationInfo::SymbolType::Variable, variable_id), CommandSource::Symbol), CommandArgument(reg, CommandSource::Register), CommandArgument(POINTER_SIZE, CommandSource::Immediate)));
					}
				}
					break;
				case Pseudo::LowLevelOpCode::STORE:
				{
					uint8_t reg0 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg0.data);
					uint8_t reg1 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg1.data);
					size_t size = operation.arg2.data;
					
					push_command(state, operation.source_line, ByteCommand(ByteOpCode::STORE_DYNAMIC, CommandArgument(reg0, CommandSource::Register), CommandArgument(reg1, CommandSource::Register), CommandArgument(size, CommandSource::Immediate)));
				}
					break;
				case Pseudo::LowLevelOpCode::LOAD:
				{
					uint8_t reg0 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg0.data);
					uint8_t reg1 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg1.data);
					size_t size = operation.arg2.data;

					push_command(state, operation.source_line, ByteCommand(ByteOpCode::LOAD_DYNAMIC, CommandArgument(reg1, CommandSource::Register), CommandArgument(reg0, CommandSource::Register), CommandArgument(size, CommandSource::Immediate)));
				}
				break;
				case Pseudo::LowLevelOpCode::ADDI:
				case Pseudo::LowLevelOpCode::SUBI:
				case Pseudo::LowLevelOpCode::MULI:
				case Pseudo::LowLevelOpCode::DIVI:
				case Pseudo::LowLevelOpCode::MODI:
				{
					uint8_t reg0 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg0.data);
					uint8_t reg1 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg1.data);
					uint8_t reg2 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg2.data);
					push_command(state, operation.source_line, ByteCommand(op_code_conversions[operation.code], CommandArgument(reg0, CommandSource::Register), CommandArgument(reg1, CommandSource::Register), CommandArgument(reg2, CommandSource::Register)));
				}
					break;

				case Pseudo::LowLevelOpCode::NEGI:

				case Pseudo::LowLevelOpCode::MOV:
				{
					uint8_t reg0 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg0.data);
					uint8_t reg1 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg1.data);
					push_command(state, operation.source_line, ByteCommand(op_code_conversions[operation.code], CommandArgument(reg0, CommandSource::Register), CommandArgument(reg1, CommandSource::Register)));
				}
					break;

				case Pseudo::LowLevelOpCode::MOVRI:
				{
					uint8_t reg0 = GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg0.data);

					push_command(state, operation.source_line, ByteCommand(op_code_conversions[operation.code], CommandArgument(reg0, CommandSource::Register), CommandArgument(operation.arg1.data, CommandSource::Immediate)));
				}
					break;

				case Pseudo::LowLevelOpCode::SPEC_CALL:
				{
					ByteCommand command;
					command.code = op_code_conversions[operation.code];
					command.arg0 = CommandArgument(operation.arg0.data, CommandSource::Immediate); //Code id

					auto args_check = [](CommandArgument& result, Pseudo::LLArgument arg, std::shared_ptr<ByteTranslationState> state, LLTranslator* translator) -> void
						{
							if (arg.source == Pseudo::LLArgumentSource::RegisterID) result = CommandArgument(translator->GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)arg.data), CommandSource::Register);
							if (arg.source == Pseudo::LLArgumentSource::Immediate) result = CommandArgument(arg.data, CommandSource::Immediate);
							if (arg.source == Pseudo::LLArgumentSource::VariableID)
							{
								if (state->dvicm[arg.data])
								{
									result = CommandArgument(state->cs_observer->GetGST().GetVariable(arg.data)->global_stack_offset, CommandSource::MemoryAddress);
								}
								else
								{
									result = CommandArgument(state->cs_observer->AddUnhandledSymbol(CompilationInfo::SymbolType::Variable, arg.data), CommandSource::Symbol);
								}
							}
						};

					args_check(command.arg1, operation.arg1, state, this);
					args_check(command.arg2, operation.arg2, state, this);

					push_command(state, operation.source_line, command);
				}
					
					break;
				case Pseudo::LowLevelOpCode::LABEL:
				{
					ByteCommand command;
					command.code = op_code_conversions[operation.code];
					command.arg0 = CommandArgument(operation.arg0.data, CommandSource::Immediate); //Label id

					push_command(state, operation.source_line, command);
				
				}
					break;

				case Pseudo::LowLevelOpCode::JMP_IF:
				case Pseudo::LowLevelOpCode::JMP_NIF:
				{
					ByteCommand command;
					command.code = op_code_conversions[operation.code];
					command.arg0 = CommandArgument(operation.arg0.data, CommandSource::Immediate); //Label id
					command.arg1 = CommandArgument(GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg1.data), CommandSource::Register); //Register id
					push_command(state, operation.source_line, command);

				}
					break;

				case Pseudo::LowLevelOpCode::JMP:
				{
					ByteCommand command;
					command.code = op_code_conversions[operation.code];
					command.arg0 = CommandArgument(operation.arg0.data, CommandSource::Immediate); //Label id

					push_command(state, operation.source_line, command);
				}
				break;

				case Pseudo::LowLevelOpCode::ALLOC:
				{
					ByteCommand command;
					command.code = op_code_conversions[operation.code];
					command.arg0 = CommandArgument(GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg0.data), CommandSource::Register);
					command.arg1 =  CommandArgument(GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg1.data), CommandSource::Register);
					command.arg2 = CommandArgument(operation.arg2.data, CommandSource::Immediate);
					push_command(state, operation.source_line, command);
				}
				break;

				case Pseudo::LowLevelOpCode::FREE:
				{
					ByteCommand command;
					command.code = op_code_conversions[operation.code];
					command.arg0 = CommandArgument(GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg0.data), CommandSource::Register);
					command.arg1 = CommandArgument(GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg1.data), CommandSource::Register);
					push_command(state, operation.source_line, command);
				}
				break;
				case Pseudo::LowLevelOpCode::COPY:
				{
					ByteCommand command;
					command.code = op_code_conversions[operation.code];
					command.arg0 = CommandArgument(GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg0.data), CommandSource::Register);
					command.arg1 = CommandArgument(GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg1.data), CommandSource::Register);
					command.arg2 = CommandArgument(GetAllocatedRegister(state, (Pseudo::LowLevelRegisters)operation.arg2.data), CommandSource::Register);
					push_command(state, operation.source_line, command);
				}
				break;

				default:
					break;
				}

			}
			void LLTranslator::FreeAllocatedRegisters(std::shared_ptr<ByteTranslationState> state)
			{
				for (auto reg : allocated_registers) 
				{
					state->registers_table.SetFree(reg.second);
				}
				allocated_registers.clear();
			}
			void LLTranslator::InsertLowLevel(std::shared_ptr<ByteTranslationState> state)
			{
				Pseudo::PseudoOperation& operation = state->pseudo_state_observer->pseudo_code[state->pseudo_ip];

				const Pseudo::LLOperationArray& array_ = state->pseudo_state_observer->ll_operations_chunks[operation.arg_0];

				for (size_t i = 0; i < array_.Size(); i++) 
				{
					HandleLowLevel(state, array_[i]);
				}
			}
		}
	}
}