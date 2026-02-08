#include "..\..\..\include\IR\ByteTranslation\CommandsHandlers.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
#pragma region Handlers


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

			ByteCommand CommandsHandler::GetConversionCommand(PrimitiveAnalogs from, PrimitiveAnalogs to, CommandArgument reg) {
				if (from == to) return ByteCommand(ByteOpCode::NOP);

				if (to == PrimitiveAnalogs::Real) {
					if (from == PrimitiveAnalogs::UInt) return ByteCommand(ByteOpCode::TC_UTR, reg);
					if (from == PrimitiveAnalogs::Int) return  ByteCommand(ByteOpCode::TC_ITR, reg);
				}
				if (to == PrimitiveAnalogs::Int) {
					if (from == PrimitiveAnalogs::UInt) return ByteCommand(ByteOpCode::TC_UTI, reg);
				}
				if (to == PrimitiveAnalogs::UInt) {
					if (from == PrimitiveAnalogs::Int) return ByteCommand(ByteOpCode::TC_ITU, reg);
				}
				return ByteCommand(ByteOpCode::NOP);
			}

			ByteCommand CommandsHandler::GetTypeConvertionCommand(PrimitiveAnalogs first, CommandArgument first_register, PrimitiveAnalogs second, CommandArgument second_register, CommandArgument& converted_register, PrimitiveAnalogs& result_type)
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

			void CommandsHandler::PushCommand(std::shared_ptr<ByteTranslationState> b_state, ByteCommand&& command, size_t line)
			{
				b_state->result.Pushback(std::move(command));
				b_state->result.Back().source_line = line;
				b_state->result.Back().pseudo_op_index = b_state->pseudo_ip;
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