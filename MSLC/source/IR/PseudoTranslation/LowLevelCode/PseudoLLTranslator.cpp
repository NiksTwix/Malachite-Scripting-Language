#include "..\..\..\..\include\IR\PseudoTranslation\LowLevelCode\PseudoLLTranslator.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			void LLTranslator::Handle(PseudoTranslationState& state, std::vector<Argument>& arguments, std::vector<Token>& op_code, size_t line)
			{
				LLOperation operation;
				operation.source_line = line;
				if (op_code.size() == 1)
				{
					LowLevelOpCode code = LLTranslationMap::Get().GetCode(op_code[0].value.strVal);
					if (code == LowLevelOpCode::NOP) goto error;
					operation.code = code;
				}
				else 
				{
					error:
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("In " + std::string(Keywords::w_op_code) + " section is invalid operation code.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, line));
						return;
				}

				switch (operation.code)
				{
				case LowLevelOpCode::LEA:
				case LowLevelOpCode::DLEA:
				{
					if (arguments.size() != 2) goto error1;

					LowLevelRegisters reg_id1 = LLTranslationMap::Get().GetRegisterID(arguments[0].tokens[0].value.strVal);

					if (arguments[1].tokens.size() > 1)
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("In " + std::string(Keywords::w_op_code) + " section complex identificator's using is forbidden.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, line));
						return;

					}
					CompilationInfo::Symbol* symbol = state.cs_observer->FindSymbolLocal(arguments[1].tokens[0].value.strVal);

					if (reg_id1 == LowLevelRegisters::InvalidR || symbol == nullptr || symbol->type != CompilationInfo::SymbolType::Variable) goto error2;


					if (operation.code == LowLevelOpCode::DLEA && !state.cs_observer->GetGST().GetVariable(symbol->description_id)->vinfo.isPointer())
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("In " + std::string(Keywords::w_op_code) + " section DLEA requires pointer variable. DLEA loads pointer which containes in variable straightaway to register. DLEA = LEA + LOAD.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, line));
						return;
					}

					operation.arg0 = LLArgument(LLArgumentSource::RegisterID, reg_id1);
					operation.arg1 = LLArgument(LLArgumentSource::VariableID, symbol->description_id);
				}
				break;
				case LowLevelOpCode::STORE:
				case LowLevelOpCode::LOAD:
				{
					if (arguments.size() != 3) goto error1;

					LowLevelRegisters reg_id1 = LLTranslationMap::Get().GetRegisterID(arguments[0].tokens[0].value.strVal);
					LowLevelRegisters reg_id2 = LLTranslationMap::Get().GetRegisterID(arguments[1].tokens[0].value.strVal);
					uint8_t size = arguments[2].tokens[0].value.intVal;
					if (reg_id1 == LowLevelRegisters::InvalidR || reg_id2 == LowLevelRegisters::InvalidR || size > MAX_VALUE_SIZE) goto error2;

					operation.arg0 = LLArgument(LLArgumentSource::RegisterID, reg_id1);
					operation.arg1 = LLArgument(LLArgumentSource::RegisterID, reg_id2);
					operation.arg2 = LLArgument(LLArgumentSource::Immediate, size);
				}
				break;

				case LowLevelOpCode::ADDI:
				case LowLevelOpCode::SUBI:
				case LowLevelOpCode::MULI:
				case LowLevelOpCode::DIVI:
				case LowLevelOpCode::MODI:
				{
					if (arguments.size() != 3) goto error1;
					LowLevelRegisters reg_id1 = LLTranslationMap::Get().GetRegisterID(arguments[0].tokens[0].value.strVal);
					LowLevelRegisters reg_id2 = LLTranslationMap::Get().GetRegisterID(arguments[1].tokens[0].value.strVal);
					LowLevelRegisters reg_id3 = LLTranslationMap::Get().GetRegisterID(arguments[2].tokens[0].value.strVal);
					if (reg_id1 == LowLevelRegisters::InvalidR || reg_id2 == LowLevelRegisters::InvalidR || reg_id3 == LowLevelRegisters::InvalidR) goto error2;
					operation.arg0 = LLArgument(LLArgumentSource::RegisterID, reg_id1);
					operation.arg1 = LLArgument(LLArgumentSource::RegisterID, reg_id2);
					operation.arg2 = LLArgument(LLArgumentSource::RegisterID, reg_id3);
				}
				break;
				case LowLevelOpCode::NEGI:
				{
					if (arguments.size() != 2) goto error1;
					LowLevelRegisters reg_id1 = LLTranslationMap::Get().GetRegisterID(arguments[0].tokens[0].value.strVal);
					LowLevelRegisters reg_id2 = LLTranslationMap::Get().GetRegisterID(arguments[1].tokens[0].value.strVal);
					if (reg_id1 == LowLevelRegisters::InvalidR || reg_id2 == LowLevelRegisters::InvalidR) goto error2;
					operation.arg0 = LLArgument(LLArgumentSource::RegisterID, reg_id1);
					operation.arg1 = LLArgument(LLArgumentSource::RegisterID, reg_id2);
				}
					break;

				case LowLevelOpCode::MOV:
				{
					if (arguments.size() != 2) goto error1;
					LowLevelRegisters reg_id1 = LLTranslationMap::Get().GetRegisterID(arguments[0].tokens[0].value.strVal);
					LowLevelRegisters reg_id2 = LLTranslationMap::Get().GetRegisterID(arguments[1].tokens[0].value.strVal);
					if (reg_id1 == LowLevelRegisters::InvalidR || reg_id2 == LowLevelRegisters::InvalidR) goto error2;
					operation.arg0 = LLArgument(LLArgumentSource::RegisterID, reg_id1);
					operation.arg1 = LLArgument(LLArgumentSource::RegisterID, reg_id2);
				}
					break;

				case LowLevelOpCode::MOVRI:
				{
					if (arguments.size() != 2) goto error1;
					if (arguments[1].tokens[0].value.type == Definitions::ValueType::STRING) goto error2;
					LowLevelRegisters reg_id1 = LLTranslationMap::Get().GetRegisterID(arguments[0].tokens[0].value.strVal);
					uint64_t value = 0;

					if (arguments[1].tokens[0].value.type == Definitions::ValueType::CHAR) value = arguments[1].tokens[0].value.charVal;
					else if (arguments[1].tokens[0].value.type == Definitions::ValueType::BOOL) value = arguments[1].tokens[0].value.boolVal;
					else 
					{
						value = arguments[1].tokens[0].value.uintVal;
					}

					if (reg_id1 == LowLevelRegisters::InvalidR) goto error2;
					operation.arg0 = LLArgument(LLArgumentSource::RegisterID, reg_id1);
					operation.arg1 = LLArgument(LLArgumentSource::Immediate, value);
				}
					break;

				case LowLevelOpCode::JMP:
				case LowLevelOpCode::JMP_IF:
				case LowLevelOpCode::JMP_NIF:
				case LowLevelOpCode::LABEL:
				{
					if (arguments.size() < 1 && arguments.size() > 2) goto error2;
					std::string argument = arguments[0].tokens[0].value.strVal;
					if (argument.empty()) goto error2;

					CompilationInfo::LabelID id = state.cs_observer->GetGST().RegisterOrGetLabelID(argument);

					operation.arg0 = LLArgument(LLArgumentSource::Immediate, id);

					if (operation.code == JMP_IF || operation.code == JMP_NIF) 
					{
						if (arguments.size() < 2) goto error2;
						LowLevelRegisters reg_id = LLTranslationMap::Get().GetRegisterID(arguments[1].tokens[0].value.strVal);
						if (reg_id == LowLevelRegisters::InvalidR) goto error2;
						operation.arg1 = LLArgument(LLArgumentSource::RegisterID, reg_id);
					}
					break;
				}
				
				case LowLevelOpCode::ALLOC:
				{
					if (arguments.size() < 2 || arguments.size() > 3) goto error2;

					LowLevelRegisters reg_id0 = LLTranslationMap::Get().GetRegisterID(arguments[0].tokens[0].value.strVal);
					LowLevelRegisters reg_id1 = LLTranslationMap::Get().GetRegisterID(arguments[1].tokens[0].value.strVal);
					if (reg_id0 == LowLevelRegisters::InvalidR || reg_id1 == LowLevelRegisters::InvalidR) goto error2;

					operation.arg0 = reg_id0;
					operation.arg1 = reg_id1;
					operation.arg2 = true;

					if (arguments.size() == 3) 
					{
						operation.arg2 = arguments[2].tokens[0].value.uintVal != 0 ? true: false;
					}

				}
					break;

				case LowLevelOpCode::FREE:
				{
					if (arguments.size() != 2) goto error2;
					LowLevelRegisters reg_id0 = LLTranslationMap::Get().GetRegisterID(arguments[0].tokens[0].value.strVal);
					LowLevelRegisters reg_id1 = LLTranslationMap::Get().GetRegisterID(arguments[1].tokens[0].value.strVal);
					if (reg_id0 == LowLevelRegisters::InvalidR || reg_id1 == LowLevelRegisters::InvalidR) goto error2;

					operation.arg0 = reg_id0;
					operation.arg1 = reg_id1;	//size
				}	
					break;
				case LowLevelOpCode::COPY:
				{
					if (arguments.size() != 3) goto error2;
					LowLevelRegisters reg_id0 = LLTranslationMap::Get().GetRegisterID(arguments[0].tokens[0].value.strVal);
					LowLevelRegisters reg_id1 = LLTranslationMap::Get().GetRegisterID(arguments[1].tokens[0].value.strVal);
					LowLevelRegisters reg_id2 = LLTranslationMap::Get().GetRegisterID(arguments[2].tokens[0].value.strVal);
					if (reg_id0 == LowLevelRegisters::InvalidR || reg_id1 == LowLevelRegisters::InvalidR || reg_id2 == LowLevelRegisters::InvalidR) goto error2;

					operation.arg0 = reg_id0;
					operation.arg1 = reg_id1;	
					operation.arg2 = reg_id2;//size
				}
				break;

				case LowLevelOpCode::SPEC_CALL: 
				{
					size_t id = arguments[0].tokens[0].value.uintVal;
					operation.arg0 = LLArgument(LLArgumentSource::Immediate, id);
					if (arguments.size() >= 2) 
					{
						LowLevelRegisters reg_id = LLTranslationMap::Get().GetRegisterID(arguments[1].tokens[0].value.strVal);
						if (reg_id != LowLevelRegisters::InvalidR) operation.arg1 = LLArgument(LLArgumentSource::RegisterID, reg_id);
						else operation.arg1 = LLArgument(LLArgumentSource::Immediate, arguments[1].tokens[0].value.uintVal);
					}
					if (arguments.size() >= 3)
					{
						LowLevelRegisters reg_id = LLTranslationMap::Get().GetRegisterID(arguments[2].tokens[0].value.strVal);
						if (reg_id != LowLevelRegisters::InvalidR) operation.arg2 = LLArgument(LLArgumentSource::RegisterID, reg_id);
						else operation.arg2 = LLArgument(LLArgumentSource::Immediate, arguments[2].tokens[0].value.uintVal);
					}
				}
					break;
				
				default:
				{
				error1:
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid \"" + op_code[0].value.strVal + "\" operation.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, line));
					return;
				error2:
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid arguments of \"" + op_code[0].value.strVal + "\" operation.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, line));
					return;
				}
				}

				state.ll_operations_chunks.back().Pushback(operation);

			}
			void LLTranslator::Translate(PseudoTranslationState& state,const AST::ASTNode& node)
			{
				size_t chunks_id = state.ll_operations_chunks.size();
				state.ll_operations_chunks.emplace_back();

				state.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::PushLLOpers, chunks_id,(uint32_t)node.line));

				for (const AST::ASTNode& cnode : node.children)
				{
					if (cnode.type == AST::ASTNodeType::None) 
					{
						continue;		
					}
					if (cnode.type != AST::ASTNodeType::Expression)
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("In " + std::string(Keywords::w_op_code) + " section are allowed only simple instructions.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, cnode.line));
						continue;
					}
					std::vector<Argument> arguments;

					std::vector<Token> op_code;
					bool arguments_ = false;

					Argument current;

					bool checking = true;

					for (const Token& t : cnode.tokens) 
					{
						if (t.value.strVal == ":") 
						{
							if (arguments_) 
							{
								Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("In " + std::string(Keywords::w_op_code) + " section repeated using ':' for arguments separating is forbidden.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, cnode.line));
								checking = false;
							}
							arguments_ = true;
							continue;
						}
						if (t.value.strVal == ",") 
						{
							if (!current.valid) 
							{
								Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("In " + std::string(Keywords::w_op_code) + " section operation contains invalid argument.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, cnode.line));
								checking = false;
								continue;
							}
							arguments.push_back(current);
							current = Argument();
							continue;
						}
						if (!arguments_) op_code.push_back(t);
						else
						{
							current.valid = true;
							current.tokens.push_back(t);
						}
					}
					if (!checking) continue;
					if (!current.valid)
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("In " + std::string(Keywords::w_op_code) + " section operation contains invalid argument.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, cnode.line));
						continue;
					}
					arguments.push_back(current);
					
					Handle(state, arguments, op_code, cnode.line);
				}
			}
		}
	}
}