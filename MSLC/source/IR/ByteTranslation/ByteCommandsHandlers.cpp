#include "..\..\..\include\IR\ByteTranslation\ByteCommandsHandlers.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
#pragma region Handlers
			void CommandsHandler::HandleMemory(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
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
								operation.debug_line);
							TryMarkAsUnhandledSymbol(frame, b_state, 0b001);
							frame.data = reg;
							frame.source = ValueSource::Pointer;  // pointer in register
							frame.dynamic_primitive_type = PrimitiveAnalogs::UInt;  // address = unsigned
							frame.dynamic_data_size = POINTER_SIZE;  // pointer size (x64)
							break;
						}

						case ValueSource::DynamicAddress:
							// Address is already in register
							frame.source = ValueSource::Pointer; 
							frame.dynamic_primitive_type = PrimitiveAnalogs::UInt;
							frame.dynamic_data_size = POINTER_SIZE;
							break;

						default:
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Getting pointer of non-memory data is invalid.", Diagnostics::LogicError, Diagnostics::SourceCode, operation.debug_line));
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
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Dereference can complited only with pointer.", Diagnostics::LogicError, Diagnostics::SourceCode, operation.debug_line));
						return;
					}

					frame.dynamic_data_size = frame.static_data_size;
					frame.source = ValueSource::DynamicAddress;
					frame.dynamic_primitive_type = frame.static_primitive_type;
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
						temp_array.Pushback(Pseudo::PseudoOperation((Pseudo::PseudoOpCode)operation.arg_0,(uint32_t)operation.debug_line));	//AL command
						size_t temp_pseudo_ip = b_state->pseudo_ip;
						b_state->pseudo_ip = 0;
						HandleAL(temp_array, b_state);
						b_state->pseudo_ip = temp_pseudo_ip;
						right = GenerateLoadCommand(p_array, b_state);	//New register value! Left we saved, right was used in operation
					}

					auto conv_cmd = GetConversionCommand(right.dynamic_primitive_type, left.dynamic_primitive_type, right.data);
					if (conv_cmd.code != ByteOpCode::NOP) {
						PushCommand(b_state, ByteCommand(conv_cmd), operation.debug_line);
					}

					if (left.source == ValueSource::StaticAddress || left.source == ValueSource::Symbol)
					{
						PushCommand(b_state, ByteCommand(ByteOpCode::STORE_STATIC,CommandArgument(left.data,CommandSource::MemoryAddress), CommandArgument(right.data, CommandSource::Register), CommandArgument(left.static_data_size, CommandSource::Immediate)), operation.debug_line);
						TryMarkAsUnhandledSymbol(left, b_state, 0b001);
					}
					else if (left.source == ValueSource::DynamicAddress) 
					{
						PushCommand(b_state, ByteCommand(ByteOpCode::STORE_DYNAMIC, CommandArgument(left.data, CommandSource::Register), CommandArgument(right.data, CommandSource::Register), CommandArgument(left.static_data_size, CommandSource::Immediate)), operation.debug_line);
					}
					else
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Assign works only with either static or dynamic addresses as left operand.", Diagnostics::DeveloperError, Diagnostics::SourceCode, operation.debug_line));
						return;
					}

					if (operation.op_code != Pseudo::PseudoOpCode::AssignR)
					{
						b_state->value_stack.pop();
						b_state->registers_table.SetFree(right.data);
					}
					break;
				}
				default:
					break;
				}
			}

			void CommandsHandler::HandleUsing(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
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
								vdesc->vinfo.isPointer() ? PrimitiveAnalogs::UInt : tdesc->primitive_analog,
								vdesc->vinfo.isPointer() ? 8 : tdesc->size));
							b_state->value_stack.top().static_primitive_type = tdesc->primitive_analog;
						}
						else 
						{
							b_state->value_stack.push(ValueFrame(ValueSource::Symbol, b_state->cs_observer->AddUnhandledSymbol(CompilationInfo::SymbolType::Variable, operation.arg_0),
								vdesc->vinfo.isPointer() ? PrimitiveAnalogs::UInt : tdesc->primitive_analog,
								vdesc->vinfo.isPointer() ? 8 : tdesc->size));
							b_state->value_stack.top().static_primitive_type = tdesc->primitive_analog;
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

			void CommandsHandler::HandleDeclaring(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
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
					b_state->frame_stack.top().size += tdesc->GetAlignedSize();
					PushCommand(b_state, ByteCommand(ByteOpCode::STACK_UP, CommandArgument(tdesc->GetAlignedSize(), CommandSource::Immediate)), operation.debug_line);
					break;
				}
				}
			}

			
			void CommandsHandler::HandleAL(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
			{
				auto operation = p_array[b_state->pseudo_ip];

				auto main_ari_handler = [&]() -> void
					{
						//Use secondly register of the one operand
						if (b_state->value_stack.size() < 2)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::IRCode, b_state->pseudo_ip));
							return;
						}
						ValueFrame right = GenerateLoadCommand(p_array,b_state);


						bool left_is_pointer = b_state->value_stack.top().source == ValueSource::Pointer;

						ValueFrame left = GenerateLoadCommand(p_array, b_state);

						size_t converted_reg = left.data;
						PrimitiveAnalogs common_type = left.dynamic_primitive_type;
						auto conv_cmd = GetTypeConvertionCommand(
							left.dynamic_primitive_type, left.data,
							right.dynamic_primitive_type, right.data,
							converted_reg, common_type
						);
						if (conv_cmd.code != ByteOpCode::NOP) {
							PushCommand(b_state, ByteCommand(conv_cmd), operation.debug_line);
						}

						if (left_is_pointer) 
						{
							auto free_register = b_state->registers_table.FindFreeGeneral();
							PushCommand(b_state, ByteCommand(			//Move data size in register
								ByteOpCode::MOVRI,
								CommandArgument(free_register, CommandSource::Register),
								CommandArgument(left.static_data_size, CommandSource::Immediate),
								CommandArgument(0, CommandSource::Immediate)
							), operation.debug_line);



							PushCommand(b_state, ByteCommand(
								GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode::Multiply, common_type),
								CommandArgument(right.data, CommandSource::Register),
								CommandArgument(right.data, CommandSource::Register),
								CommandArgument(free_register, CommandSource::Register)
							), operation.debug_line);
							b_state->registers_table.SetFree(free_register);

							
						}

						PushCommand(b_state, ByteCommand(
							GetTypedArithmeticCommandCode(operation.op_code, common_type),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(right.data, CommandSource::Register)
						), operation.debug_line);
						b_state->registers_table.SetFree(right.data);

						b_state->value_stack.push(ValueFrame(left.source,left.data,common_type, converted_reg == left.data ? left.dynamic_data_size : right.dynamic_data_size));
					};
				auto main_logic_handler = [&]() -> void
					{
						if (b_state->value_stack.size() < 2)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::IRCode, b_state->pseudo_ip));
							return;
						}
						ValueFrame right = GenerateLoadCommand(p_array, b_state);
						ValueFrame left = GenerateLoadCommand(p_array, b_state);

						PushCommand(b_state, 
							ByteCommand(GetLogicCommand(operation.op_code, PrimitiveAnalogs::UInt), CommandArgument(left.data, CommandSource::Register), 
								CommandArgument(left.data, CommandSource::Register), CommandArgument(right.data, CommandSource::Register)), operation.debug_line);

						b_state->registers_table.SetFree(right.data);
						b_state->value_stack.push(ValueFrame(ValueSource::Register, left.data, PrimitiveAnalogs::UInt, LOGIC_RESULT_SIZE));	//1 BYTE
					};
				auto main_bit_logic_handler = [&]() -> void
					{
						if (b_state->value_stack.size() < 2)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::IRCode, b_state->pseudo_ip));
							return;
						}
						ValueFrame right = GenerateLoadCommand(p_array, b_state);
						ValueFrame left = GenerateLoadCommand(p_array, b_state);
	
						if (left.dynamic_primitive_type == PrimitiveAnalogs::Real || right.dynamic_primitive_type == PrimitiveAnalogs::Real) {
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Bit operation requires integer types.", Diagnostics::MessageType::LogicError, Diagnostics::SourceType::SourceCode, operation.debug_line));
							return;
						}

						PushCommand(b_state, ByteCommand(
							GetLogicCommand(operation.op_code, PrimitiveAnalogs::UInt),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(right.data, CommandSource::Register)
						), operation.debug_line);

						b_state->registers_table.SetFree(right.data);
						b_state->value_stack.push(ValueFrame(ValueSource::Register, left.data, PrimitiveAnalogs::UInt, left.dynamic_data_size));
					};
				auto main_cmp_handler = [&]() -> void
					{
						if (b_state->value_stack.size() < 2)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::IRCode, b_state->pseudo_ip));
							return;
						}
						ValueFrame right = GenerateLoadCommand(p_array, b_state);
						ValueFrame left = GenerateLoadCommand(p_array, b_state);

						size_t converted_reg;
						PrimitiveAnalogs common_type;
						auto conv_cmd = GetTypeConvertionCommand(
							left.dynamic_primitive_type, left.data,
							right.dynamic_primitive_type, right.data,
							converted_reg, common_type
						);
						if (conv_cmd.code != ByteOpCode::NOP) {
							PushCommand(b_state, ByteCommand(conv_cmd), operation.debug_line);	
						}

						PushCommand(b_state,
							ByteCommand(GetLogicCommand(operation.op_code, common_type), CommandArgument(left.data, CommandSource::Register),
								CommandArgument(left.data, CommandSource::Register), CommandArgument(right.data, CommandSource::Register)), operation.debug_line);

						b_state->registers_table.SetFree(right.data);
						b_state->value_stack.push(ValueFrame(ValueSource::Register, left.data, PrimitiveAnalogs::UInt, LOGIC_RESULT_SIZE));	// 1 BYTE
					};
				switch (operation.op_code)
				{
				case Pseudo::PseudoOpCode::Add:
				case Pseudo::PseudoOpCode::Subtract:
				case Pseudo::PseudoOpCode::Divide:
				case Pseudo::PseudoOpCode::Exponentiate:
				case Pseudo::PseudoOpCode::Multiply:
					main_ari_handler();
					break;
				
				case Pseudo::PseudoOpCode::And:
					main_logic_handler();
					break;
				case Pseudo::PseudoOpCode::Or:
					main_logic_handler();
					break;
				case Pseudo::PseudoOpCode::Not:
					break;
				case Pseudo::PseudoOpCode::Mod:
				{
					if (b_state->value_stack.size() < 2)
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::IRCode, b_state->pseudo_ip));
						return;
					}
					ValueFrame right = GenerateLoadCommand(p_array, b_state);
					ValueFrame left = GenerateLoadCommand(p_array, b_state);

					// Mod is only for integer types
					if (left.dynamic_primitive_type == PrimitiveAnalogs::Real || right.dynamic_primitive_type == PrimitiveAnalogs::Real) {
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Mod operation requires integer types.", Diagnostics::MessageType::LogicError, Diagnostics::SourceType::SourceCode, operation.debug_line));
						return;
					}
					size_t converted_reg;
					PrimitiveAnalogs common_type;
					auto conv_cmd = GetTypeConvertionCommand(
						left.dynamic_primitive_type, left.data,
						right.dynamic_primitive_type, right.data,
						converted_reg, common_type
					);
					if (conv_cmd.code != ByteOpCode::NOP) {
						PushCommand(b_state, ByteCommand(conv_cmd), operation.debug_line);
					}

					PushCommand(b_state, ByteCommand(
						GetTypedArithmeticCommandCode(operation.op_code, common_type),
						CommandArgument(left.data, CommandSource::Register),
						CommandArgument(left.data, CommandSource::Register),
						CommandArgument(right.data, CommandSource::Register)
					), operation.debug_line);
					b_state->registers_table.SetFree(right.data);
					//ValueFrame(left.source_arg, common_type, converted_reg.reg_index == left.source_arg.reg_index ? left.data_size : right.data_size)

					b_state->value_stack.push(ValueFrame(left.source, left.data, common_type, converted_reg == left.data ? left.dynamic_data_size : right.dynamic_data_size));
					break;
				}
				case Pseudo::PseudoOpCode::Negative:
				{
					if (b_state->value_stack.size() < 1)
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Negative is unary operation, but gets zero arguments.", Diagnostics::MessageType::LogicError, Diagnostics::SourceType::SourceCode, operation.debug_line));
						break;
					}
					ValueFrame vf_left = GenerateLoadCommand(p_array,b_state);

					// Negative only for int or double/real (INT, DOUBLE)
					if (vf_left.dynamic_primitive_type == PrimitiveAnalogs::UInt) {
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Negative operation requires signed types (int, real).", Diagnostics::MessageType::TypeError, Diagnostics::SourceType::SourceCode, operation.debug_line));
						break;
					}

					PushCommand(b_state, ByteCommand(
						GetTypedArithmeticCommandCode(operation.op_code, vf_left.dynamic_primitive_type),
						CommandArgument( vf_left.data, CommandSource::Register),
						CommandArgument( vf_left.data, CommandSource::Register)), operation.debug_line);
					b_state->value_stack.push(vf_left);
					break;
				}
				case Pseudo::PseudoOpCode::PrefixIncrement:
					break;
				case Pseudo::PseudoOpCode::PrefixDecrement:
					break;
				case Pseudo::PseudoOpCode::PostfixDecrement:
					break;
				case Pseudo::PseudoOpCode::PostfixIncrement:
					break;
				case Pseudo::PseudoOpCode::BitNot:
				{
					
					ValueFrame vf_left = GenerateLoadCommand(p_array, b_state);

					PushCommand(b_state, ByteCommand(
						GetLogicCommand(operation.op_code, vf_left.dynamic_primitive_type),
						CommandArgument(vf_left.data, CommandSource::Register),
						CommandArgument(vf_left.data, CommandSource::Register)), operation.debug_line);
					b_state->value_stack.push(vf_left);
					break;
				}

				case Pseudo::PseudoOpCode::BitOr:
				case Pseudo::PseudoOpCode::BitAnd:
				case Pseudo::PseudoOpCode::BitOffsetLeft:
				case Pseudo::PseudoOpCode::BitOffsetRight:
					main_bit_logic_handler();
					break;

				case Pseudo::PseudoOpCode::Equal:
				case Pseudo::PseudoOpCode::NotEqual:
				case Pseudo::PseudoOpCode::Greater:
				case Pseudo::PseudoOpCode::Less:
				case Pseudo::PseudoOpCode::GreaterEqual:
				case Pseudo::PseudoOpCode::LessEqual:
					main_cmp_handler();
					break;
				default:
					break;
				}
			}

#pragma endregion

			ValueFrame CommandsHandler::GenerateLoadCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
			{
				if (b_state->value_stack.empty())
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Values stack is empty.", Diagnostics::DeveloperError, Diagnostics::IRCode, b_state->pseudo_ip));
					return ValueFrame::Invalid();
				}
				size_t current_line = p_array[b_state->pseudo_ip].debug_line;
				ValueFrame frame = b_state->value_stack.top(); b_state->value_stack.pop();

				if (frame.source == ValueSource::Register || frame.source == ValueSource::Pointer) return frame;

				if (frame.source == ValueSource::DynamicAddress) {
					// Address is already in register  (reg_index)
					if (frame.dynamic_data_size > 8) {
						// Big data — leave a pointer (address in the register)
						frame.source = ValueSource::Pointer;
						frame.dynamic_data_size = 8;
						return frame;
					}
					else {
						// Small data — load values by address 
						// LOAD_DYNAMIC [reg_addr] → reg_value
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LOAD_DYNAMIC, CommandArgument(frame.data, CommandSource::Register),                    // register with address
								CommandArgument(frame.data, CommandSource::Register), // destination / Register reusing
								CommandArgument(frame.dynamic_data_size, Immediate)),
							current_line);

						frame.source = ValueSource::Register;
						return frame;
					}
				}
				else if (frame.source == ValueSource::StaticAddress || frame.source == ValueSource::Symbol) {
					// Static address (offset)
					size_t free_reg = b_state->registers_table.AllocateFreeGeneral();

					if (!CheckRegister(b_state, free_reg))return ValueFrame::Invalid();

					if (frame.dynamic_data_size > 8) {
						// Big data — load address
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LEA_STATIC,
								CommandArgument(frame.data, CommandSource::MemoryAddress),                    // static_offset
								CommandArgument(free_reg, Register)),
							current_line);
						TryMarkAsUnhandledSymbol(frame, b_state,0b1);
						frame.data = free_reg;
						frame.source = ValueSource::Pointer;
						frame.dynamic_data_size = 8;
						
					}
					else {
						// Small data — load value
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LOAD_STATIC,
								CommandArgument(frame.data, CommandSource::MemoryAddress),                  // static_offset
								CommandArgument(free_reg, Register),
								CommandArgument(frame.dynamic_data_size, Immediate)),
							current_line);
						TryMarkAsUnhandledSymbol(frame, b_state, 0b1);
						frame.data = free_reg;
						frame.source = ValueSource::Register;
					}
					return frame;
				}
				else if (frame.source == ValueSource::Constant)
				{
					size_t free_reg = b_state->registers_table.AllocateFreeGeneral();
					if (!CheckRegister(b_state, free_reg))return ValueFrame::Invalid();
					if (frame.dynamic_data_size > 8) {
						// Big data — load address of constant
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LEA_CONST,
								CommandArgument(frame.data, CommandSource::Constant),                   
								CommandArgument(free_reg, Register)),
							current_line);
						frame.data = free_reg;
						frame.source = ValueSource::Pointer;
						frame.dynamic_data_size = 8;
					}
					else {
						// Small data — load value
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LOAD_CONST_STATIC,
								CommandArgument(frame.data, CommandSource::Constant),                   // static_offset
								CommandArgument(free_reg, Register),
								CommandArgument(frame.dynamic_data_size, Immediate)),
							current_line);
						frame.data = free_reg;
						frame.source = ValueSource::Register;
					}
				}

				
				//Another will be in the future
				return frame;
			}

			

			ByteOpCode CommandsHandler::GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode code, PrimitiveAnalogs type)
			{
				if (type == PrimitiveAnalogs::Int)
				{
					switch (code)
					{
					case Pseudo::PseudoOpCode::Add:
						return ByteOpCode::ADDI;
					case Pseudo::PseudoOpCode::Subtract:
						return ByteOpCode::SUBI;
					case Pseudo::PseudoOpCode::Multiply:
						return ByteOpCode::MULI;
					case Pseudo::PseudoOpCode::Divide:
						return ByteOpCode::DIVI;
					case Pseudo::PseudoOpCode::Mod:
						return ByteOpCode::MODI;
					case Pseudo::PseudoOpCode::Negative:
						return ByteOpCode::NEGI;
					case Pseudo::PseudoOpCode::Exponentiate:
						return ByteOpCode::EXPI;
					}
				}
				if (type == PrimitiveAnalogs::UInt)
				{
					switch (code)
					{
					case Pseudo::PseudoOpCode::Add:
						return ByteOpCode::ADDU;
					case Pseudo::PseudoOpCode::Subtract:
						return ByteOpCode::SUBU;
					case Pseudo::PseudoOpCode::Multiply:
						return ByteOpCode::MULU;
					case Pseudo::PseudoOpCode::Divide:
						return ByteOpCode::DIVU;
					case Pseudo::PseudoOpCode::Mod:
						return ByteOpCode::MODU;
					case Pseudo::PseudoOpCode::Exponentiate:
						return ByteOpCode::EXPU;
					}
				}
				if (type == PrimitiveAnalogs::Real)
				{
					switch (code)
					{
					case Pseudo::PseudoOpCode::Add:
						return ByteOpCode::ADDR;
					case Pseudo::PseudoOpCode::Subtract:
						return ByteOpCode::SUBR;
					case Pseudo::PseudoOpCode::Multiply:
						return ByteOpCode::MULR;
					case Pseudo::PseudoOpCode::Divide:
						return ByteOpCode::DIVR;
					case Pseudo::PseudoOpCode::Negative:
						return ByteOpCode::NEGR;
					case Pseudo::PseudoOpCode::Exponentiate:
						return ByteOpCode::EXPR;
					}
				}
				return ByteOpCode::NOP;
			}

			ByteCommand CommandsHandler::GetConversionCommand(PrimitiveAnalogs from, PrimitiveAnalogs to, size_t reg) {
				if (from == to) return ByteCommand(ByteOpCode::NOP);

				if (to == PrimitiveAnalogs::Real) {
					if (from == PrimitiveAnalogs::UInt) return ByteCommand(ByteOpCode::TC_UTR, CommandArgument(reg,CommandSource::Register));
					if (from == PrimitiveAnalogs::Int) return  ByteCommand(ByteOpCode::TC_ITR, CommandArgument(reg, CommandSource::Register));
				}
				if (to == PrimitiveAnalogs::Int) {
					if (from == PrimitiveAnalogs::UInt) return ByteCommand(ByteOpCode::TC_UTI, CommandArgument(reg, CommandSource::Register));
					if (from == PrimitiveAnalogs::Real) return ByteCommand(ByteOpCode::TC_RTI, CommandArgument(reg, CommandSource::Register));
				}
				if (to == PrimitiveAnalogs::UInt) {
					if (from == PrimitiveAnalogs::Int) return ByteCommand(ByteOpCode::TC_ITU, CommandArgument(reg, CommandSource::Register));
					if (from == PrimitiveAnalogs::Real) return ByteCommand(ByteOpCode::TC_RTU, CommandArgument(reg, CommandSource::Register));
				}
				return ByteCommand(ByteOpCode::NOP);
			}

			ByteCommand CommandsHandler::GetTypeConvertionCommand(PrimitiveAnalogs first, size_t first_register, PrimitiveAnalogs second, size_t second_register, size_t& converted_register, PrimitiveAnalogs& result_type)
			{
				PrimitiveAnalogs target_type = (first > second) ? first : second;	//Define target type
				if (first != target_type) {
					converted_register = first_register;
					result_type = target_type;
					return GetConversionCommand(first, target_type, first_register);
				}
				else {
					converted_register = second_register;
					result_type = target_type;
					return GetConversionCommand(second, target_type, second_register);
				}
			}

			PrimitiveAnalogs CommandsHandler::ValueContainerTypeToPrimitive(Definitions::ValueType type)
			{
				switch (type)
				{
				case MSLC::Definitions::ValueType::VOID:
				case MSLC::Definitions::ValueType::UINT:
				case MSLC::Definitions::ValueType::BOOL:
					return PrimitiveAnalogs::UInt;
				case MSLC::Definitions::ValueType::INT:
				case MSLC::Definitions::ValueType::CHAR:
					return PrimitiveAnalogs::Int;
				case MSLC::Definitions::ValueType::REAL:
					return PrimitiveAnalogs::Real;
				case MSLC::Definitions::ValueType::STRING:
					break;
				default:
					break;
				}
				return PrimitiveAnalogs::UInt;
			}

			ByteOpCode CommandsHandler::GetLogicCommand(Pseudo::PseudoOpCode code, PrimitiveAnalogs type)
			{
				switch (code)
				{
				case Pseudo::PseudoOpCode::And:
					return ByteOpCode::AND;
				case Pseudo::PseudoOpCode::Or:
					return ByteOpCode::OR;
				case Pseudo::PseudoOpCode::Not:
					return ByteOpCode::NOT;
				case Pseudo::PseudoOpCode::BitOr:
					return ByteOpCode::BIT_OR;
				case Pseudo::PseudoOpCode::BitNot:
					return ByteOpCode::BIT_NOT;
				case Pseudo::PseudoOpCode::BitAnd:
					return ByteOpCode::BIT_AND;
				case Pseudo::PseudoOpCode::BitOffsetLeft:
					return ByteOpCode::BIT_OFFSET_LEFT;
				case Pseudo::PseudoOpCode::BitOffsetRight:
					return ByteOpCode::BIT_OFFSET_RIGHT;
				case Pseudo::PseudoOpCode::Equal:
				case Pseudo::PseudoOpCode::NotEqual:
				case Pseudo::PseudoOpCode::Greater:
				case Pseudo::PseudoOpCode::Less:
				case Pseudo::PseudoOpCode::GreaterEqual:
				case Pseudo::PseudoOpCode::LessEqual:
					if (type == PrimitiveAnalogs::Int)return ByteOpCode::CMPI;
					if (type == PrimitiveAnalogs::UInt)return ByteOpCode::CMPU;
					if (type == PrimitiveAnalogs::Real)return ByteOpCode::CMPR;
					break;
				default:
					break;
				}
				return ByteOpCode::NOP;
			}

			void CommandsHandler::PushCommand(std::shared_ptr<ByteTranslationState> b_state, ByteCommand&& command, size_t line)
			{
				b_state->result.Pushback(std::move(command));
				b_state->result.Back().source_line = line;
				b_state->result.Back().pseudo_op_index = b_state->pseudo_ip;
			}

			bool CommandsHandler::CheckRegister(std::shared_ptr<ByteTranslationState> b_state, size_t register_)
			{
				if (register_ == InvalidRegister) 
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid allocated register.", Diagnostics::DeveloperError, Diagnostics::IRCode, b_state->pseudo_ip));
					return false;
				}
				return true;
			}

			void CommandsHandler::TryMarkAsUnhandledSymbol(ValueFrame& frame, std::shared_ptr<ByteTranslationState> b_state, uint8_t args_with_us)
			{
				if (frame.source == ValueSource::Symbol)
				{
					b_state->result.Back().flags |= ByteCommand::Flag::UnhandledSymbol;
					if (args_with_us & (1 << 0)) b_state->result.Back().arg0.type = CommandSource::Symbol;
					if (args_with_us & (1 << 1)) b_state->result.Back().arg1.type = CommandSource::Symbol;
					if (args_with_us & (1 << 2)) b_state->result.Back().arg2.type = CommandSource::Symbol;
				}
			}

			void CommandsHandler::HandleCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
			{
				Pseudo::PseudoOperation& operation = p_array.Get(b_state->pseudo_ip);

				if (operation.op_code == Pseudo::PseudoOpCode::PushFrame)
				{
					b_state->PushFrame(); 
					return;
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

				if (operation.op_code > Pseudo::PseudoOpCode::ST_AL && operation.op_code < Pseudo::PseudoOpCode::ED_AL) 
				{
					HandleAL(p_array, b_state);
				}
				else if (operation.op_code > Pseudo::PseudoOpCode::ST_MEM && operation.op_code < Pseudo::PseudoOpCode::ED_MEM)
				{
					HandleMemory(p_array, b_state);
				}
				else if (operation.op_code > Pseudo::PseudoOpCode::ST_USING && operation.op_code < Pseudo::PseudoOpCode::ED_USING) 
				{
					HandleUsing(p_array, b_state);
				}
				else if (operation.op_code > Pseudo::PseudoOpCode::ST_DECL && operation.op_code < Pseudo::PseudoOpCode::ED_DEC)
				{
					HandleDeclaring(p_array, b_state);
				}
			}
		}
	}
}