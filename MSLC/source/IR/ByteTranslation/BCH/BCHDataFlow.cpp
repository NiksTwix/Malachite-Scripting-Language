#include "..\..\..\..\include\IR\ByteTranslation\BCH\BCHDataFlow.hpp"


namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			namespace BCH
			{
				void DataFlowHandler::HandleMemory(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state, ArithmeticLogicHandler& alh)
				{
					auto operation = p_array[b_state->pseudo_ip];

					switch (operation.op_code)
					{
					case Pseudo::PseudoOpCode::GetPointer:
					{
						if (b_state->value_stack.empty())
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Values stack is empty.", Diagnostics::DeveloperError, Diagnostics::IRCode, b_state->pseudo_ip));
							return;
						}
						ValueFrame frame = b_state->value_stack.top(); b_state->value_stack.pop();

						switch (frame.source) {
						case ValueSource::Symbol:
						case ValueSource::StaticAddress:
						{
							size_t reg = b_state->registers_table.AllocateFreeGeneral();
							PushCommand(b_state,
								ByteCommand(ByteOpCode::LEA_STATIC,
									CommandArgument(frame.data, CommandSource::MemoryAddress),
									CommandArgument(reg, CommandSource::Register)),
								b_state->current_line);
							TryMarkAsUnhandledSymbol(frame, b_state, 0b001);
							frame.data = reg;
							frame.GetPointer();	//value source changed here
							break;
						}

						case ValueSource::DynamicAddress:
							// Address is already in register
							frame.GetPointer();
							break;

						default:
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Getting pointer of non-memory data is invalid.", Diagnostics::LogicError, Diagnostics::SourceCode, b_state->current_line));
							return;
						}
						b_state->value_stack.push(frame);
						break;
					}

					case Pseudo::PseudoOpCode::Dereference:
					{
						if (b_state->value_stack.empty())
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Values stack is empty.", Diagnostics::DeveloperError, Diagnostics::IRCode, b_state->pseudo_ip));
							return;
						}
						ValueFrame frame = b_state->value_stack.top();

						if (frame.source == ValueSource::StaticAddress || frame.source == ValueSource::DynamicAddress || frame.source == ValueSource::Symbol) {
							// Generate LOAD_STATIC/LOAD_DYNAMIC that get Pointer in register
							frame = GenerateLoadCommand(p_array, b_state);  // now Pointer
						}
						else b_state->value_stack.pop();	//GLC didnt be called therefore we should pop it by hand

						if (frame.source != ValueSource::Pointer && frame.source != ValueSource::Register)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Dereference can be complited only with pointer.", Diagnostics::LogicError, Diagnostics::SourceCode, b_state->current_line));
							return;
						}

						frame.Dereference();

						if (frame.value_info.isPointer())	//If dereferenced value is pointer we need to load it because in another case it just will be switching POINTER/DYNAMIC_ADDRESS. 
							//If we remove this check pointer with depth 1 will be loaded two times (here and by another operation) pointer->value->??? 
						{
							PushCommand(b_state,
								ByteCommand(ByteOpCode::LOAD_DYNAMIC, CommandArgument(frame.data, CommandSource::Register),                    // register with address
									CommandArgument(frame.data, CommandSource::Register), // destination / Register reusing
									CommandArgument(frame.dynamic_data_size, Immediate)),
								b_state->current_line);
						}
						


						b_state->value_stack.push(frame);
						break;
					}
					case Pseudo::PseudoOpCode::Assign:
					case Pseudo::PseudoOpCode::AssignR:		//R = return (result) - pushs to value stack result of operation: y = x = z + 1
					{
						if (b_state->value_stack.size() < 2)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Assign demands two operands.", Diagnostics::DeveloperError, Diagnostics::IRCode, b_state->pseudo_ip));
							return;
						}
						ValueFrame right = GenerateLoadCommand(p_array, b_state);
						ValueFrame left = b_state->value_stack.top();


						//For not-primitive types will be applied construction call (in the TypeChecker)
						if (operation.arg_0 != 0)
						{
							//Arithmetic and complex assignment: x += 1

							b_state->value_stack.push(right);	//left wasnt removed

							Pseudo::POperationArray temp_array;
							temp_array.Pushback(Pseudo::PseudoOperation((Pseudo::PseudoOpCode)operation.arg_0, (uint32_t)b_state->current_line));	//AL command
							size_t temp_pseudo_ip = b_state->pseudo_ip;
							b_state->pseudo_ip = 0;
							alh.Handle(temp_array, b_state);
							b_state->pseudo_ip = temp_pseudo_ip;
							right = GenerateLoadCommand(p_array, b_state);
							if (operation.op_code == Pseudo::PseudoOpCode::AssignR)
							{
								b_state->value_stack.push(left);
								/*
									When we took values from stack right has been removed and left has been saved.
									When we computed complex assign left and right have been removed from stack by arithmetic/logic operation (loading operation)
									We should return left value frame to stack for AssignR working
								*/
							}
						}

						if (!(left.value_info.isPointer() && right.dynamic_primitive_type == PrimitiveAnalogs::UInt) || (left.value_info.isPointer() && right.dynamic_primitive_type == PrimitiveAnalogs::Int))
						{
							auto conv_cmd = GetConversionCommand(right.dynamic_primitive_type, left.dynamic_primitive_type, right.data);
							if (conv_cmd.code != ByteOpCode::NOP) {
								PushCommand(b_state, ByteCommand(conv_cmd), b_state->current_line);
							}
						}
						else if (left.value_info.isPointer() && (right.dynamic_primitive_type != PrimitiveAnalogs::UInt && right.dynamic_primitive_type != PrimitiveAnalogs::Int))
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Saving not uint (or negative int) value as pointer is forbidden.", Diagnostics::TypeError, Diagnostics::SourceCode, b_state->current_line));
							return;
						}


						if (left.source == ValueSource::StaticAddress || left.source == ValueSource::Symbol)
						{
							PushCommand(b_state, ByteCommand(ByteOpCode::STORE_STATIC, CommandArgument(left.data, CommandSource::MemoryAddress), CommandArgument(right.data, CommandSource::Register), CommandArgument(left.static_data_size, CommandSource::Immediate)), b_state->current_line);
							TryMarkAsUnhandledSymbol(left, b_state, 0b001);
						}
						else if (left.source == ValueSource::DynamicAddress)
						{
							PushCommand(b_state, ByteCommand(ByteOpCode::STORE_DYNAMIC, CommandArgument(left.data, CommandSource::Register), CommandArgument(right.data, CommandSource::Register), CommandArgument(left.static_data_size, CommandSource::Immediate)), b_state->current_line);
						}
						else
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Assign works only with either static or dynamic addresses as left operand.", Diagnostics::DeveloperError, Diagnostics::SourceCode, b_state->current_line));
							return;
						}

						if (operation.op_code != Pseudo::PseudoOpCode::AssignR)
						{
							if (operation.arg_0 == 0)b_state->value_stack.pop();	//WARN
							//If was complex assign -> left and right was deleted and arguments for this command have out
							b_state->registers_table.SetFree(right.data);
						}
						break;

					}
					case Pseudo::PseudoOpCode::GetByArgument:	//Offset mode
					{

						if (b_state->value_stack.size() < 2)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("GetByArgument demands two operands.", Diagnostics::DeveloperError, Diagnostics::IRCode, b_state->pseudo_ip));
							return;
						}
						ValueFrame right = GenerateLoadCommand(p_array, b_state);	//index
						ValueFrame left = GenerateLoadCommand(p_array, b_state);		//identifier (data in the memory cell)



						if (left.source != ValueSource::Pointer && left.source != ValueSource::Register)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("GetByArgument works only with either pointer or register as left operand.", Diagnostics::DeveloperError, Diagnostics::SourceCode, b_state->current_line));
							return;
						}
						if (left.value_info.type_id != CompilationInfo::INVALID_ID)
						{
							PushPointerArithmetic(left, right, b_state, right.dynamic_primitive_type);
						}


						PushCommand(b_state, ByteCommand(
							GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode::Add, right.dynamic_primitive_type),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(right.data, CommandSource::Register)
						), b_state->current_line);
						b_state->registers_table.SetFree(right.data);

						
						//Generate load command
						left.Dereference();
						
						if (left.value_info.isPointer())	//If dereferenced value is pointer we need to load it because in another case it just will be switching POINTER/DYNAMIC_ADDRESS. 
							//If we remove this check pointers with depth 1 will be loaded two times (here and by another operation) pointer->value->??? (dont ask the questions. Just forget. It works- and its proper)
						{
							PushCommand(b_state,
								ByteCommand(ByteOpCode::LOAD_DYNAMIC, CommandArgument(left.data, CommandSource::Register),                    // register with address
									CommandArgument(left.data, CommandSource::Register), // destination / Register reusing
									CommandArgument(left.dynamic_data_size, Immediate)),
								b_state->current_line);
						}
						b_state->value_stack.push(left);
						break;
					}
					default:
						break;
					}
				}

				void DataFlowHandler::HandleUsing(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
				{
					auto operation = p_array[b_state->pseudo_ip];

					switch (operation.op_code)
					{
					case Pseudo::PseudoOpCode::Use:
					{
						CompilationInfo::Variables::VariableDescription* vdesc = b_state->cs_observer->GetGST().GetVariable(operation.arg_0);
						CompilationInfo::Types::TypeDescription* tdesc = b_state->cs_observer->GetGST().GetType(vdesc->vinfo.type_id);

						if (b_state->dvicm[operation.arg_0])
						{
							b_state->value_stack.push(ValueFrame(ValueSource::StaticAddress, vdesc->global_stack_offset,
								tdesc->primitive_analog,
								vdesc->vinfo.isPointer() ? POINTER_SIZE : tdesc->size));
							b_state->value_stack.top().state = b_state->cs_observer;
							b_state->value_stack.top().SetValueInfo(vdesc->vinfo);
							
						}
						else
						{
							b_state->value_stack.push(ValueFrame(ValueSource::Symbol, b_state->cs_observer->AddUnhandledSymbol(CompilationInfo::SymbolType::Variable, operation.arg_0),
								tdesc->primitive_analog,
								vdesc->vinfo.isPointer() ? POINTER_SIZE : tdesc->size));
							b_state->value_stack.top().state = b_state->cs_observer;
							b_state->value_stack.top().SetValueInfo(vdesc->vinfo);
						
						}

						break;
					}
					case Pseudo::PseudoOpCode::UseConstant:
					{
						const Definitions::ValueContainer& constant = b_state->cs_observer->GetICT().GetByID(operation.arg_0);
						b_state->value_stack.push(ValueFrame(ValueSource::Constant, operation.arg_0, ValueContainerTypeToPrimitive(constant.type), constant.GetDataSize()));
						break;
					}
					}
				}

				void DataFlowHandler::HandleDeclaring(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
				{
					auto operation = p_array[b_state->pseudo_ip];

					switch (operation.op_code)
					{
					case Pseudo::PseudoOpCode::DeclareVariable:
					{
						b_state->dvicm[operation.arg_0] = true;
						CompilationInfo::Variables::VariableDescription* vdesc = b_state->cs_observer->GetGST().GetVariable(operation.arg_0);
						CompilationInfo::Types::TypeDescription* tdesc = b_state->cs_observer->GetGST().GetType(vdesc->vinfo.type_id);

						vdesc->global_stack_offset = b_state->frame_stack.top().NextFreeAddress();

						size_t size = vdesc->vinfo.isPointer() ? POINTER_SIZE : tdesc->GetAlignedSize();

						b_state->frame_stack.top().size += size;
						PushCommand(b_state, ByteCommand(ByteOpCode::STACK_UP, CommandArgument(size, CommandSource::Immediate)), b_state->current_line);
						break;
					}
					}
				}
			}
		}
	}
}