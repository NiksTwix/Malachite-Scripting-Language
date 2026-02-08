#include "..\..\..\include\IR\ByteTranslation\CommandsHandlers.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
#pragma region Handlers
			void CommandsHandler::HandleArithmetic(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
			{
				Pseudo::PseudoOperation& operation = p_array[b_state->pseudo_ip];

				auto main_ari_handler = [&]() -> void
					{
						//Use secondly register of the one operand
						if (b_state->value_stack.size() < 2 )
						{
							//Logger::Get().PrintTypeError(SyntaxInfo::GetPseudoString(cmd.op_code) + " is binary operation, but gets only one.", current_BDS.ip);
							return;
						}
						ValueFrame right = b_state->value_stack.top(); b_state->value_stack.pop();
						ValueFrame left = b_state->value_stack.top(); b_state->value_stack.pop();

						CommandArgument converted_reg = left.source_arg;
						ValueFrame::ValueNativeType common_type = left.native_type;
						auto conv_cmd = GetTypeConvertionCommand(
							left.native_type, left.source_arg,
							right.native_type, right.source_arg,
							converted_reg, common_type
						);
						if (conv_cmd.code != ByteOpCode::NOP) {
							b_state->result.Pushback(conv_cmd);
						}

						b_state->result.Pushback(ByteCommand(
							GetTypedArithmeticCommandCode(operation.op_code, common_type),
							left.source_arg,
							left.source_arg,
							right.source_arg
						));

						b_state->registers_table.SetFree(right.source_arg.reg_index);
						b_state->value_stack.push(ValueFrame(left.source_arg, common_type));
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
				default:
					break;
				}

			}

#pragma endregion


			ByteOpCode CommandsHandler::GetTypedArithmeticCommandCode(Pseudo::PseudoOpCode code, ValueFrame::ValueNativeType type)
			{
				if (type == ValueFrame::ValueNativeType::Int)
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
				if (type == ValueFrame::ValueNativeType::UInt)
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
				if (type == ValueFrame::ValueNativeType::Real)
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

			ByteCommand CommandsHandler::GetConversionCommand(ValueFrame::ValueNativeType from, ValueFrame::ValueNativeType to, CommandArgument reg) {
				if (from == to) return ByteCommand(ByteOpCode::NOP);

				if (to == ValueFrame::ValueNativeType::Real) {
					if (from == ValueFrame::ValueNativeType::UInt) return ByteCommand(ByteOpCode::TC_UTR, reg);
					if (from == ValueFrame::ValueNativeType::Int) return  ByteCommand(ByteOpCode::TC_ITR, reg);
				}
				if (to == ValueFrame::ValueNativeType::Int) {
					if (from == ValueFrame::ValueNativeType::UInt) return ByteCommand(ByteOpCode::TC_UTI, reg);
				}
				if (to == ValueFrame::ValueNativeType::UInt) {
					if (from == ValueFrame::ValueNativeType::Int) return ByteCommand(ByteOpCode::TC_ITU, reg);
				}
				return ByteCommand(ByteOpCode::NOP);
			}

			ByteCommand CommandsHandler::GetTypeConvertionCommand(ValueFrame::ValueNativeType first, CommandArgument first_register, ValueFrame::ValueNativeType second, CommandArgument second_register, CommandArgument& converted_register, ValueFrame::ValueNativeType& result_type)
			{
				ValueFrame::ValueNativeType  target_type = (first > second) ? first : second;	//Define target type
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

			void CommandsHandler::HandleCommand(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState> b_state)
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
		}
	}
}