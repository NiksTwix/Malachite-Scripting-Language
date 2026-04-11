#include "..\..\..\..\include\IR\ByteTranslation\BCH\BCHUtils.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			namespace BCH
			{
				ByteOpCode GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode code, PrimitiveAnalogs type)
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

				ByteCommand GetConversionCommand(PrimitiveAnalogs from, PrimitiveAnalogs to, size_t reg) {
					if (from == to) return ByteCommand(ByteOpCode::NOP);

					if (to == PrimitiveAnalogs::Real) {
						if (from == PrimitiveAnalogs::UInt) return ByteCommand(ByteOpCode::TC_UTR, CommandArgument(reg, CommandSource::Register));
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

				ByteCommand GetTypeConvertionCommand(PrimitiveAnalogs first, size_t first_register, PrimitiveAnalogs second, size_t second_register, size_t& converted_register, PrimitiveAnalogs& result_type)
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

				PrimitiveAnalogs ValueContainerTypeToPrimitive(Definitions::ValueType type)
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

				ByteOpCode GetLogicCommand(Pseudo::PseudoOpCode code, PrimitiveAnalogs type)
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
						if (type == PrimitiveAnalogs::Int)return ByteOpCode::EQUALI;
						if (type == PrimitiveAnalogs::UInt)return ByteOpCode::EQUALU;
						if (type == PrimitiveAnalogs::Real)return ByteOpCode::EQUALR;
					case Pseudo::PseudoOpCode::NotEqual:
						if (type == PrimitiveAnalogs::Int)return ByteOpCode::NOT_EQUALI;
						if (type == PrimitiveAnalogs::UInt)return ByteOpCode::NOT_EQUALU;
						if (type == PrimitiveAnalogs::Real)return ByteOpCode::NOT_EQUALR;
					case Pseudo::PseudoOpCode::Greater:
						if (type == PrimitiveAnalogs::Int)return ByteOpCode::GREATERI;
						if (type == PrimitiveAnalogs::UInt)return ByteOpCode::GREATERU;
						if (type == PrimitiveAnalogs::Real)return ByteOpCode::GREATERR;
					case Pseudo::PseudoOpCode::Less:
						if (type == PrimitiveAnalogs::Int)return ByteOpCode::LESSI;
						if (type == PrimitiveAnalogs::UInt)return ByteOpCode::LESSU;
						if (type == PrimitiveAnalogs::Real)return ByteOpCode::LESSR;
					case Pseudo::PseudoOpCode::GreaterEqual:
						if (type == PrimitiveAnalogs::Int)return ByteOpCode::EGREATERI;
						if (type == PrimitiveAnalogs::UInt)return ByteOpCode::EGREATERU;
						if (type == PrimitiveAnalogs::Real)return ByteOpCode::EGREATERR;
					case Pseudo::PseudoOpCode::LessEqual:
						if (type == PrimitiveAnalogs::Int)return ByteOpCode::ELESSI;
						if (type == PrimitiveAnalogs::UInt)return ByteOpCode::ELESSU;
						if (type == PrimitiveAnalogs::Real)return ByteOpCode::ELESSR;
						break;
					default:
						break;
					}
					return ByteOpCode::NOP;
				}

				void PushCommand(std::shared_ptr<ByteTranslationState> b_state, ByteCommand&& command, size_t line)
				{
					b_state->result.Pushback(std::move(command));
					b_state->result.Back().source_line = line;
					b_state->result.Back().pseudo_op_index = b_state->pseudo_ip;
				}

				bool CheckRegister(std::shared_ptr<ByteTranslationState> b_state, size_t register_)
				{
					if (register_ == InvalidRegister)
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid allocated register.", Diagnostics::DeveloperError, Diagnostics::SourceCode, b_state->GetDeclaringPlace()));
						return false;
					}
					return true;
				}

				void TryMarkAsUnhandledSymbol(ValueFrame& frame, std::shared_ptr<ByteTranslationState> b_state, uint8_t args_with_us)
				{
					if (frame.source == ValueSource::Symbol)
					{
						b_state->result.Back().flags |= ByteCommand::Flag::UnhandledSymbol;
						if (args_with_us & (1 << 0)) b_state->result.Back().arg0.type = CommandSource::Symbol;
						if (args_with_us & (1 << 1)) b_state->result.Back().arg1.type = CommandSource::Symbol;
						if (args_with_us & (1 << 2)) b_state->result.Back().arg2.type = CommandSource::Symbol;
					}
				}

				void PushPointerArithmetic(ValueFrame& left, ValueFrame& right, std::shared_ptr<ByteTranslationState> b_state, PrimitiveAnalogs type)
				{
					if (left.dynamic_primitive_type == PrimitiveAnalogs::Real || right.dynamic_primitive_type == PrimitiveAnalogs::Real)
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Pointer arithmetic with real numbers is forbidden.", Diagnostics::TypeError, Diagnostics::SourceCode, b_state->GetDeclaringPlace()));
					}

					auto aligned_size = b_state->cs_observer->GetICT().GetOrAdd(left.GetTypeDesc(left.value_info.type_id, b_state->cs_observer)->GetAlignedSize());

					auto free_register = b_state->registers_table.FindFreeGeneral();
					PushCommand(b_state, ByteCommand(			//Move data size in register
						ByteOpCode::LOAD_CONST_STATIC,
						CommandArgument(aligned_size, CommandSource::Constant),
						CommandArgument(free_register, CommandSource::Register),
						CommandArgument(sizeof(left.static_data_size), CommandSource::Immediate)
					), b_state->GetDeclaringPlace().place);



					PushCommand(b_state, ByteCommand(
						GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode::Multiply, type),
						CommandArgument(right.data, CommandSource::Register),
						CommandArgument(right.data, CommandSource::Register),
						CommandArgument(free_register, CommandSource::Register)
					), b_state->GetDeclaringPlace().place);
					b_state->registers_table.SetFree(free_register);

				}
				ValueFrame GenerateLoadCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
				{
					if (b_state->value_stack.empty())
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Values stack is empty.", Diagnostics::DeveloperError, Diagnostics::SourceCode, b_state->GetDeclaringPlace()));
						return ValueFrame::Invalid();
					}
					size_t current_line = p_array[b_state->pseudo_ip].declaring_place.place;
					ValueFrame frame = b_state->value_stack.top(); b_state->value_stack.pop();

					if (frame.source == ValueSource::Register || frame.source == ValueSource::Pointer) return frame;

					
					if (frame.source == ValueSource::DynamicAddress) {

						if (frame.dynamic_data_size > POINTER_SIZE)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Attempt of loading big data(>8 bytes) to register", Diagnostics::DeveloperError, Diagnostics::SourceCode, b_state->GetDeclaringPlace()));
						}

						// LOAD_DYNAMIC [reg_addr] → reg_value
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LOAD_DYNAMIC, CommandArgument(frame.data, CommandSource::Register),                    // register with address
								CommandArgument(frame.data, CommandSource::Register), // destination / Register reusing
								CommandArgument(frame.dynamic_data_size, Immediate)),
							current_line);
						
						frame.source = ValueSource::Register;
						return frame;

					}
					else if (frame.source == ValueSource::StaticAddress || frame.source == ValueSource::Symbol) {
						// Static address (offset)
						size_t free_reg = b_state->registers_table.AllocateFreeGeneral();

						if (!CheckRegister(b_state, free_reg))return ValueFrame::Invalid();

						if (frame.dynamic_data_size > POINTER_SIZE)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Attempt of loading big data(>8 bytes) to register", Diagnostics::DeveloperError, Diagnostics::SourceCode, b_state->GetDeclaringPlace()));
						}

						PushCommand(b_state,
							ByteCommand(ByteOpCode::LOAD_STATIC,
								CommandArgument(frame.data, CommandSource::MemoryAddress),                  // static_offset
								CommandArgument(free_reg, Register),
								CommandArgument(frame.dynamic_data_size, Immediate)),
							current_line);
						TryMarkAsUnhandledSymbol(frame, b_state, 0b1);
						frame.data = free_reg;
					
						frame.source = frame.value_info.isPointer() ? ValueSource::Pointer : ValueSource::Register;
						frame.dynamic_primitive_type = frame.source == ValueSource::Pointer ? PrimitiveAnalogs::UInt : frame.static_primitive_type;	//WARN
						return frame;
					}

					else if (frame.source == ValueSource::Constant)
					{
						size_t free_reg = b_state->registers_table.AllocateFreeGeneral();
						if (!CheckRegister(b_state, free_reg))return ValueFrame::Invalid();
						if (frame.dynamic_data_size > POINTER_SIZE || b_state->cs_observer->GetICT().GetByID(frame.data).type == Definitions::ValueType::STRING) {
							// Big data — load address of constant
							PushCommand(b_state,
								ByteCommand(ByteOpCode::LEA_CONST,
									CommandArgument(frame.data, CommandSource::Constant),
									CommandArgument(free_reg, Register)),
								current_line);
							frame.data = free_reg;
							frame.source = ValueSource::Pointer;
							frame.dynamic_data_size = POINTER_SIZE;
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
			}
		}
	}
}
