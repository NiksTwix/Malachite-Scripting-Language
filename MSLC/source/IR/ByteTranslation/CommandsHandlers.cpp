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