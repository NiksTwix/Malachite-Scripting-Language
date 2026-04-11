#include "..\..\..\..\include\IR\ByteTranslation\BCH\BCHAL.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			namespace BCH
			{
				void ArithmeticLogicHandler::Handle(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
				{
					auto operation = p_array[b_state->pseudo_ip];

					auto add_converted_frame = [](std::shared_ptr<ByteTranslationState> b_state, ValueFrame& left, ValueFrame& right, size_t converted_reg, PrimitiveAnalogs common_type) -> void
						{
							ValueFrame new_frame = ValueFrame(left.source, left.data, common_type, converted_reg == left.data ? left.dynamic_data_size : right.dynamic_data_size, b_state->cs_observer);
							new_frame.static_primitive_type = converted_reg == left.data ? right.static_primitive_type : left.static_primitive_type;
							new_frame.static_data_size = converted_reg == left.data ? right.static_data_size :  left.static_data_size;
							new_frame.value_info = converted_reg == left.data ? right.value_info : left.value_info;
							//We must take value_info and another from source register (to which we convert data) if converted right - we take left, otherwise right
							b_state->value_stack.push(new_frame);
						};


					auto main_ari_handler = [&]() -> void
						{
							//Use secondly register of the one operand
							if (b_state->value_stack.size() < 2)
							{
								Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::SourceCode, b_state->GetDebugInfo()));
								return;
							}
							ValueFrame right = GenerateLoadCommand(p_array, b_state);

							ValueFrame left = GenerateLoadCommand(p_array, b_state);
							bool left_is_pointer = left.source == ValueSource::Pointer;

							size_t converted_reg = left.data;
							PrimitiveAnalogs common_type = left.dynamic_primitive_type;
							auto conv_cmd = GetTypeConvertionCommand(
								left.dynamic_primitive_type, left.data,
								right.dynamic_primitive_type, right.data,
								converted_reg, common_type
							);
							if (conv_cmd.code != ByteOpCode::NOP) {
								PushCommand(b_state, ByteCommand(conv_cmd), b_state->GetDebugInfo().place);
							}

							if (left_is_pointer && left.value_info.type_id != CompilationInfo::INVALID_ID)
							{
								PushPointerArithmetic(left, right, b_state, common_type);
							}

							PushCommand(b_state, ByteCommand(
								GetTypedArithmeticCommandCode(operation.op_code, common_type),
								CommandArgument(left.data, CommandSource::Register),
								CommandArgument(left.data, CommandSource::Register),
								CommandArgument(right.data, CommandSource::Register)
							), b_state->GetDebugInfo().place);
							b_state->registers_table.SetFree(right.data);

							add_converted_frame(b_state, left, right, converted_reg, common_type);
						};
					auto main_logic_handler = [&]() -> void
						{
							if (b_state->value_stack.size() < 2)
							{
								Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::SourceCode, b_state->GetDebugInfo()));
								return;
							}
							ValueFrame right = GenerateLoadCommand(p_array, b_state);
							ValueFrame left = GenerateLoadCommand(p_array, b_state);

							PushCommand(b_state,
								ByteCommand(GetLogicCommand(operation.op_code, PrimitiveAnalogs::UInt), CommandArgument(left.data, CommandSource::Register),
									CommandArgument(left.data, CommandSource::Register), CommandArgument(right.data, CommandSource::Register)), b_state->GetDebugInfo().place);

							b_state->registers_table.SetFree(right.data);
							b_state->value_stack.push(ValueFrame(ValueSource::Register, left.data, PrimitiveAnalogs::UInt, LOGIC_RESULT_SIZE, b_state->cs_observer));	//1 BYTE
						};
					auto main_bit_logic_handler = [&]() -> void
						{
							if (b_state->value_stack.size() < 2)
							{
								Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::SourceCode, b_state->GetDebugInfo()));
								return;
							}
							ValueFrame right = GenerateLoadCommand(p_array, b_state);
							ValueFrame left = GenerateLoadCommand(p_array, b_state);

							if (left.dynamic_primitive_type == PrimitiveAnalogs::Real || right.dynamic_primitive_type == PrimitiveAnalogs::Real) {
								Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Bit operation requires integer types.", Diagnostics::MessageType::LogicError, Diagnostics::SourceType::SourceCode, b_state->GetDebugInfo()));
								return;
							}

							PushCommand(b_state, ByteCommand(
								GetLogicCommand(operation.op_code, PrimitiveAnalogs::UInt),
								CommandArgument(left.data, CommandSource::Register),
								CommandArgument(left.data, CommandSource::Register),
								CommandArgument(right.data, CommandSource::Register)
							), b_state->GetDebugInfo().place);

							b_state->registers_table.SetFree(right.data);
							b_state->value_stack.push(ValueFrame(ValueSource::Register, left.data, PrimitiveAnalogs::UInt, left.dynamic_data_size, b_state->cs_observer));
						};
					auto main_cmp_handler = [&]() -> void
						{
							if (b_state->value_stack.size() < 2)
							{
								Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::SourceCode, b_state->GetDebugInfo()));
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
								PushCommand(b_state, ByteCommand(conv_cmd), b_state->GetDebugInfo().place);
							}

							PushCommand(b_state,
								ByteCommand(GetLogicCommand(operation.op_code, common_type), CommandArgument(left.data, CommandSource::Register),
									CommandArgument(left.data, CommandSource::Register), CommandArgument(right.data, CommandSource::Register)), b_state->GetDebugInfo().place);

							b_state->registers_table.SetFree(right.data);
							b_state->value_stack.push(ValueFrame(ValueSource::Register, left.data, PrimitiveAnalogs::UInt, LOGIC_RESULT_SIZE, b_state->cs_observer));	// 1 BYTE
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
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Binary operation gets less than two operands.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::SourceCode, b_state->GetDebugInfo()));
							return;
						}
						ValueFrame right = GenerateLoadCommand(p_array, b_state);
						ValueFrame left = GenerateLoadCommand(p_array, b_state);

						// Mod is only for integer types
						if (left.dynamic_primitive_type == PrimitiveAnalogs::Real || right.dynamic_primitive_type == PrimitiveAnalogs::Real) {
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Mod operation requires integer types.", Diagnostics::MessageType::LogicError, Diagnostics::SourceType::SourceCode, b_state->GetDebugInfo()));
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
							PushCommand(b_state, ByteCommand(conv_cmd), b_state->GetDebugInfo().place);
						}

						PushCommand(b_state, ByteCommand(
							GetTypedArithmeticCommandCode(operation.op_code, common_type),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(right.data, CommandSource::Register)
						), b_state->GetDebugInfo().place);
						b_state->registers_table.SetFree(right.data);
						//ValueFrame(left.source_arg, common_type, converted_reg.reg_index == left.source_arg.reg_index ? left.data_size : right.data_size)

						add_converted_frame(b_state, left, right, converted_reg, common_type);
						break;
					}
					case Pseudo::PseudoOpCode::Negative:
					{
						if (b_state->value_stack.size() < 1)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Negative is unary operation, but gets zero arguments.", Diagnostics::MessageType::LogicError, Diagnostics::SourceType::SourceCode, b_state->GetDebugInfo()));
							break;
						}
						ValueFrame vf_left = GenerateLoadCommand(p_array, b_state);

						// Negative only for int or double/real (INT, DOUBLE)
						if (vf_left.dynamic_primitive_type == PrimitiveAnalogs::UInt) {
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Negative operation requires signed types (int, real).", Diagnostics::MessageType::TypeError, Diagnostics::SourceType::SourceCode, b_state->GetDebugInfo()));
							break;
						}

						PushCommand(b_state, ByteCommand(
							GetTypedArithmeticCommandCode(operation.op_code, vf_left.dynamic_primitive_type),
							CommandArgument(vf_left.data, CommandSource::Register),
							CommandArgument(vf_left.data, CommandSource::Register)), b_state->GetDebugInfo().place);
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
							CommandArgument(vf_left.data, CommandSource::Register)), b_state->GetDebugInfo().place);
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
			}
		}
	}
}