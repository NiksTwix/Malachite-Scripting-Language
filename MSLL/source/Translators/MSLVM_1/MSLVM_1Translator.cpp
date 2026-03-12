#pragma once
#include "..\..\..\include\Translators\MSLVM_1\MSLVM_1Translator.hpp"


namespace MSLL
{
	namespace MSLVM_1
	{
		bool TranslatorVM_1::HandleCommand(std::shared_ptr<ObjectsInfo::CommandsPool> commands, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result, size_t& i)
		{

			ObjectsInfo::ByteCommand& cmd = commands->commands[i];

			//Symbol handling

			if (cmd.flags & ObjectsInfo::Flag::UnhandledSymbol) 
			{
				//checking arg0,arg1,arg2 -> replace symbol to address or ip by symbol's type
				auto checking = [](ObjectsInfo::CommandArgument& arg, std::shared_ptr<ObjectsInfo::LinkingState> state, size_t i)
					{
						if (arg.type == ObjectsInfo::CommandSource::Symbol)
						{
							if (arg.data >= state->symbols.size())
							{
								std::cerr << "Invalid symbol in argument 0. Byte Command index:" << i << "\n";
								return false;
							}
							auto& symbol_info = state->symbols[arg.data];
							arg.data = state->stack_offset_of_module[symbol_info.module_id] + symbol_info.offset_m_c;
							arg.type = symbol_info.type == ObjectsInfo::SymbolType::Variable ? ObjectsInfo::CommandSource::MemoryAddress : ObjectsInfo::CommandSource::Immediate;
						}
					};
				checking(cmd.arg0, state, i);
				checking(cmd.arg1, state, i);
				checking(cmd.arg2, state, i);
			}

			bool check = true;

			if ((cmd.code > ObjectsInfo::ByteOpCode::SECTION_ARITHMETIC_ST && cmd.code < ObjectsInfo::ByteOpCode::SECTION_ARITHMETIC_ED) 
				|| (cmd.code > ObjectsInfo::ByteOpCode::SECTION_LOGIC_ST && cmd.code < ObjectsInfo::ByteOpCode::SECTION_LOGIC_ED))
			{
				check = check && HandleAL(cmd, state, result);
			}
			if (cmd.code > ObjectsInfo::ByteOpCode::SECTION_CONTROL_FLOW_ST && cmd.code < ObjectsInfo::ByteOpCode::SECTION_CONTROL_FLOW_ED)
			{
				check = check && HandleAL(cmd, state, result);
			}
			if (cmd.code > ObjectsInfo::ByteOpCode::SECTION_MEMORY_ST && cmd.code < ObjectsInfo::ByteOpCode::SECTION_MEMORY_ED)
			{
				check = check && HandleMR(cmd, state, result);
			}
			else 
			{
				switch (cmd.code)
				{
				case ObjectsInfo::ByteOpCode::NOP:
					break;
				case ObjectsInfo::ByteOpCode::SYMBOL_LABEL:
					break;
				default:
					break;
				}
			}

			return check;
		}

		bool TranslatorVM_1::HandleAL(ObjectsInfo::ByteCommand command, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result)
		{
			VMOperationCode code = OpCodeTable::Get().At(command.code);
			if (code == VMOperationCode::NOP) 
			{
				std::cerr << "Unsupported operation. Byte operation code:" << command.code << "\n";
				return false;
			}
			
			//Direct order of arguments

			VMOperation operation;
			operation.code = code;
			operation.arg0 = command.arg0.data;
			operation.arg1 = command.arg1.data;
			operation.arg2 = command.arg2.data;
			//command.source_line
			result.push_back(operation);
			return true;
		}

		bool TranslatorVM_1::HandleCF(ObjectsInfo::ByteCommand command, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result)
		{
			return false;
		}

		bool TranslatorVM_1::HandleMR(ObjectsInfo::ByteCommand command, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result)
		{
			switch (command.code)
			{
			case ObjectsInfo::ByteOpCode::MOVRR://Unused
				break;
			case ObjectsInfo::ByteOpCode::MOVRI://Unused
				break;
			case ObjectsInfo::ByteOpCode::STACK_UP://Direct order of arguments
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = SpecialRegister::A0;
				operation.arg1 = command.arg0;
				result.push_back(operation);
				operation.code = VMOperationCode::ADD_RRR_UNSIGNED;
				operation.arg0 = SpecialRegister::SP;
				operation.arg1 = SpecialRegister::SP;
				operation.arg2 = SpecialRegister::A0;
				result.push_back(operation);
			}
				break;
			case ObjectsInfo::ByteOpCode::STACK_DOWN:
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = SpecialRegister::A0;
				operation.arg1 = command.arg0;
				result.push_back(operation);
				operation.code = VMOperationCode::SUB_RRR_UNSIGNED;
				operation.arg0 = SpecialRegister::SP;
				operation.arg1 = SpecialRegister::SP;
				operation.arg2 = SpecialRegister::A0;
				result.push_back(operation);
			}
				break;
			case ObjectsInfo::ByteOpCode::LEA_STATIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				result.push_back(operation);
				break;
			}	
			case ObjectsInfo::ByteOpCode::LEA_DYNAMIC:
				std::cerr << "Unsupported operation. Byte operation code:" << command.code << "\n";
				return false;

			case ObjectsInfo::ByteOpCode::LOAD_DYNAMIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::LOAD_BY_ADDRESS;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				operation.arg2 = command.arg2.data;
				result.push_back(operation);
				break;
			}	
			case ObjectsInfo::ByteOpCode::STORE_DYNAMIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::STORE_BY_ADDRESS;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				operation.arg2 = command.arg2.data;
				result.push_back(operation);
				break;
			}
			case ObjectsInfo::ByteOpCode::LOAD_CONST_STATIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::LOAD_CONST_LOCAL;
				operation.arg0 = command.arg1.data;
				operation.arg1 = state->constants[command.arg0.data].memory_offset;
				operation.arg2 = command.arg2.data;
				result.push_back(operation);
				break;
			}
			case ObjectsInfo::ByteOpCode::LEA_CONST:
			{
				std::cerr << "Unsupported operation. Byte operation code:" << command.code << "\n";
				return false;
			}
			case ObjectsInfo::ByteOpCode::LOAD_STATIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::LOAD_LOCAL;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				operation.arg2 = command.arg2.data;
				result.push_back(operation);
				break;
			}
			case ObjectsInfo::ByteOpCode::STORE_STATIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::STORE_LOCAL;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				operation.arg2 = command.arg2.data;
				result.push_back(operation);
				break;
			}

			default:
				break;
			}
			return false;
		}


		ObjectsInfo::ExecutionData TranslatorVM_1::Translate(fs::path directory, std::shared_ptr<ObjectsInfo::LinkingState> state, ObjectsReader& reader)
		{
			ObjectsInfo::ExecutionData execution_data;

			execution_data.read_only_data.allocate(state->constants_size);

			//moving data, constants freeing and offset calculating

			size_t c_offset = 0;

			for (auto& constant: state->constants)
			{
				constant.memory_offset = c_offset;

				if (constant.size_in_bytes + c_offset > execution_data.read_only_data.bytes_size) 
				{
					std::cerr << "Invalid size calculating in the constants handling process.";
					execution_data.Free();
					state->FreeConstants();
					return ObjectsInfo::ExecutionData();
				}

				memcpy(execution_data.read_only_data.ptr + c_offset, constant.data, constant.size_in_bytes); 
				c_offset += constant.size_in_bytes;
				constant.Free();
			}

			state->global_stack_offset = c_offset;

			std::vector<VMOperation> commands;
			for (ObjectsInfo::moduleid id : state->linking_order) 
			{
				std::string file_name = state->module_prefix + std::to_string(id) + "." + state->module_extention;
				state->stack_offset_of_module[id] = state->global_stack_offset;
				bool check = HandleModule(directory / file_name, state, reader, commands);
				if (!check) 
				{
					std::cerr << "Error of module(" << id << ") handling. Module's name:" << file_name << "\n";
					return ObjectsInfo::ExecutionData();
				}
			}


			return execution_data;
		}
		bool TranslatorVM_1::HandleModule(fs::path file, std::shared_ptr<ObjectsInfo::LinkingState> state, ObjectsReader& reader, std::vector<VMOperation>& commands)
		{
			ObjectsInfo::static_bpointer  co_bytes = reader.ReadFile(file.string());
			if (co_bytes.ptr == nullptr)
			{
				std::cerr << "Invalid CO file \"" + file.string() + "\".\n";
				return false;
			}

			std::shared_ptr<ObjectsInfo::CommandsPool> pool = reader.DeserializeCO(co_bytes);
			

			for (size_t i = 0; i < pool->commands.size(); i++) 
			{
				if (HandleCommand(pool, state, commands, i)) return false;
			}

			state->global_stack_offset += pool->stack_size;
			return true;
		}
	}
}