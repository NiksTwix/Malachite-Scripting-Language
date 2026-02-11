#include "..\..\..\include\IR\ByteTranslation\CommandsHandlers.hpp"

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
				case Pseudo::PseudoOpCode::Use:
					{
					CompilationInfo::Variables::VariableDescription* vdesc = b_state->cs_observer->GetGST().GetVariable(operation.arg_0);
					CompilationInfo::Types::TypeDescription* tdesc = b_state->cs_observer->GetGST().GetType(vdesc->vinfo.type_id);
					b_state->value_stack.push(ValueFrame(ValueSource::StaticAddress, vdesc->global_stack_offset,
						vdesc->vinfo.isPointer() ? PrimitiveAnalogs::UInt : tdesc->primitive_analog, 
						vdesc->vinfo.isPointer() ? 8 : tdesc->size));
					break;
					}
				case Pseudo::PseudoOpCode::UseConstant:
				{
					const Definitions::ValueContainer& constant = b_state->cs_observer->GetICT().GetByID(operation.arg_0);
					b_state->value_stack.push(ValueFrame(ValueSource::Constant, operation.arg_0, PrimitiveAnalogs::UInt,constant.GetDataSize()));
					break;
				}
				default:
					break;
				}
			}

			ValueFrame CommandsHandler::GenerateLoadCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
			{
				if (b_state->value_stack.empty()) 
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Values stack is empty.", Diagnostics::DeveloperError, Diagnostics::IRCode, b_state->pseudo_ip));
					return ValueFrame::Invalid();
				}
				size_t current_line = p_array[b_state->pseudo_ip].debug_line;
				ValueFrame frame = b_state->value_stack.top(); b_state->value_stack.pop();
				if (frame.source== ValueSource::DynamicAddress) {
					// Address is already in register  (reg_index)
					if (frame.data_size > 8) {
						// Big data — leave a pointer (address in the register)
						frame.source = ValueSource::Register;
						return frame;
					}
					else {
						// Small data — load values by address 
						// LOAD_DYNAMIC [reg_addr] → reg_value
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LOAD_DYNAMIC,CommandArgument(frame.data,CommandSource::Register),                    // register with address
								CommandArgument(frame.data, CommandSource::Register), // destination / Register reusing
								CommandArgument(frame.data_size, Immediate)),
							current_line);

						frame.source = ValueSource::Register;
						return frame;
					}
				}
				else if (frame.source == ValueSource::StaticAddress) {
					// Static address (offset)
					size_t free_reg = b_state->registers_table.FindFreeGeneral();
					if (CheckRegister(b_state, free_reg))return ValueFrame::Invalid();

					if (frame.data_size > 8) {
						// Big data — load address
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LEA_STATIC,
								CommandArgument(frame.data,CommandSource::MemoryAddress),                    // static_offset
								CommandArgument(free_reg, Register)),
							current_line);
						frame.data = free_reg;
						frame.source = ValueSource::Register;
						//frame.data_size = 8; 
					}
					else {
						// Small data — load value
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LOAD_STATIC,
								CommandArgument(frame.data, CommandSource::MemoryAddress),                  // static_offset
								CommandArgument(free_reg, Register),
								CommandArgument(frame.data_size, Immediate)),
							current_line);
						frame.data = free_reg;
						frame.source = ValueSource::Register;

					}
					return frame;
				}
				else if (frame.source ==  ValueSource::Constant)
				{
					size_t free_reg = b_state->registers_table.FindFreeGeneral();
					if (CheckRegister(b_state, free_reg))return ValueFrame::Invalid();
					if (frame.data_size > 8) {
						// Big data — load address
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LOAD_CONST_DYNAMIC,
								CommandArgument(frame.data, CommandSource::Constant),                   // static_offset
								CommandArgument(free_reg, Register)),
							current_line);
						frame.data = free_reg;
						frame.source = ValueSource::Register;
					}
					else {
						// Small data — load value
						PushCommand(b_state,
							ByteCommand(ByteOpCode::LOAD_CONST_STATIC,
								CommandArgument(frame.data, CommandSource::Constant),                   // static_offset
								CommandArgument(free_reg, Register),
								CommandArgument(frame.data_size, Immediate)),
							current_line);
						frame.data = free_reg;
						frame.source = ValueSource::Register;
					}
				}

				//Another will be in the future
				return frame;
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
						ValueFrame left = GenerateLoadCommand(p_array, b_state);

						size_t converted_reg = left.data;
						PrimitiveAnalogs common_type = left.native_type;
						size_t size = left.data_size;
						auto conv_cmd = GetTypeConvertionCommand(
							left.native_type, left.data,
							right.native_type, right.data,
							converted_reg, common_type
						);
						if (conv_cmd.code != ByteOpCode::NOP) {
							PushCommand(b_state, conv_cmd, operation.debug_line);	//reference becomes invalid
						}

						PushCommand(b_state, ByteCommand(
							GetTypedArithmeticCommandCode(operation.op_code, common_type),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(left.data, CommandSource::Register),
							CommandArgument(right.data, CommandSource::Register)
						), operation.debug_line);
						b_state->registers_table.SetFree(right.data);
						//ValueFrame(left.source_arg, common_type, converted_reg.reg_index == left.source_arg.reg_index ? left.data_size : right.data_size)

						b_state->value_stack.push(ValueFrame(ValueSource::Register,left.data,common_type, converted_reg == left.data ? left.data_size : right.data_size));
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
						b_state->value_stack.push(ValueFrame(ValueSource::Register, left.data, PrimitiveAnalogs::UInt,left.data_size));
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
							left.native_type, left.data,
							right.native_type, right.data,
							converted_reg, common_type
						);
						if (conv_cmd.code != ByteOpCode::NOP) {
							PushCommand(b_state, conv_cmd, operation.debug_line);	//reference becomes invalid
						}

						PushCommand(b_state,
							ByteCommand(GetLogicCommand(operation.op_code, PrimitiveAnalogs::UInt), CommandArgument(left.data, CommandSource::Register),
								CommandArgument(left.data, CommandSource::Register), CommandArgument(right.data, CommandSource::Register)), operation.debug_line);

						b_state->registers_table.SetFree(right.data);
						b_state->value_stack.push(ValueFrame(ValueSource::Register, left.data, PrimitiveAnalogs::UInt, left.data_size));
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
					break;
				case Pseudo::PseudoOpCode::Negative:
					break;
				case Pseudo::PseudoOpCode::PrefixIncrement:
					break;
				case Pseudo::PseudoOpCode::PrefixDecrement:
					break;
				case Pseudo::PseudoOpCode::PostfixDecrement:
					break;
				case Pseudo::PseudoOpCode::PostfixIncrement:
					break;
				case Pseudo::PseudoOpCode::BitNot:
					break;
				case Pseudo::PseudoOpCode::BitOr:
					main_logic_handler();
					break;
				
				break;
				case Pseudo::PseudoOpCode::BitAnd:
					main_logic_handler();
					break;
				case Pseudo::PseudoOpCode::BitOffsetLeft:
					main_logic_handler();
					break;
				case Pseudo::PseudoOpCode::BitOffsetRight:
					main_logic_handler();
					break;

					//Comparings

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
				}
				if (to == PrimitiveAnalogs::UInt) {
					if (from == PrimitiveAnalogs::Int) return ByteCommand(ByteOpCode::TC_ITU, CommandArgument(reg, CommandSource::Register));
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
			void CommandsHandler::PushCommand(std::shared_ptr<ByteTranslationState> b_state, ByteCommand& command, size_t line)
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



			}
		}
	}
}