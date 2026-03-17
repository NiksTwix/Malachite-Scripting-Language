#pragma once
#include "..\..\..\include\Translators\MSLVM_1\MSLVM_1Translator.hpp"

#include <string>

namespace MSLL
{
	namespace MSLVM_1
	{
		bool TranslatorVM_1::HandleCommand(std::shared_ptr<LinkDefinitions::CommandsPool> commands, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result, size_t& i)
		{

			LinkDefinitions::ByteCommand& cmd = commands->commands[i];

			//Symbol handling

			//if (cmd.flags & LinkDefinitions::Flag::UnhandledSymbol) 
			//{
			//	//checking arg0,arg1,arg2 -> replace symbol to address or ip by symbol's type
			//	
			//}
			auto checking = [](LinkDefinitions::CommandArgument& arg, std::shared_ptr<LinkDefinitions::LinkingState> state, size_t i, LinkDefinitions::moduleid m_id) -> void
				{
					if (arg.type == LinkDefinitions::CommandSource::Symbol)
					{
						if (arg.data >= state->symbols.size())
						{
							std::cerr << "Invalid symbol in argument 0. Byte Command index:" << i << "\n";

						}
						auto& symbol_info = state->symbols[arg.data];
						if (symbol_info.type == LinkDefinitions::SymbolType::Variable)
						{
							arg.data = state->stack_offset_of_module[symbol_info.module_id] + symbol_info.offset_m_c;
							arg.type = LinkDefinitions::CommandSource::MemoryAddress;
						}
						else if (symbol_info.type == LinkDefinitions::SymbolType::Function)
						{
							arg.data = symbol_info.native_code_offset;
							arg.type = LinkDefinitions::CommandSource::Immediate;
						}
					}
					else if (arg.type == LinkDefinitions::CommandSource::MemoryAddress)
					{
						arg.data = state->stack_offset_of_module[m_id] + arg.data;
					}
					else if (arg.type == LinkDefinitions::CommandSource::Constant)
					{
						arg.data = state->constants[arg.data].memory_offset;
						arg.type = LinkDefinitions::CommandSource::MemoryAddress;
					}
				};
			checking(cmd.arg0, state, i,commands->m_id);
			checking(cmd.arg1, state, i,commands->m_id);
			checking(cmd.arg2, state, i,commands->m_id);
			bool check = true;

			if ((cmd.code > LinkDefinitions::ByteOpCode::SECTION_ARITHMETIC_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_ARITHMETIC_ED) 
				|| (cmd.code > LinkDefinitions::ByteOpCode::SECTION_LOGIC_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_LOGIC_ED))
			{
				check = check && HandleAL(cmd, state, result);
			}
			if (cmd.code > LinkDefinitions::ByteOpCode::SECTION_CONTROL_FLOW_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_CONTROL_FLOW_ED)
			{
				check = check && HandleAL(cmd, state, result);
			}
			if (cmd.code > LinkDefinitions::ByteOpCode::SECTION_MEMORY_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_MEMORY_ED)
			{
				check = check && HandleMR(cmd, state, result);
			}
			else 
			{
				switch (cmd.code)
				{
				case LinkDefinitions::ByteOpCode::NOP:
					break;
				case LinkDefinitions::ByteOpCode::SYMBOL_LABEL:
					break;
				default:
					break;
				}
			}

			return check;
		}

		bool TranslatorVM_1::HandleAL(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result)
		{
			VMOperationCode code = OpCodeTable::Get().At(command.code);
			if (code == VMOperationCode::NOP) 
			{
				std::cerr << "Unsupported operation. Byte operation code:" << (uint8_t)command.code << "\n";
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

		bool TranslatorVM_1::HandleCF(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result)
		{
			return false;
		}

		bool TranslatorVM_1::HandleMR(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result)
		{
			switch (command.code)
			{
			case LinkDefinitions::ByteOpCode::MOVRR://Unused
				break;
			case LinkDefinitions::ByteOpCode::MOVRI://Unused
				break;
			case LinkDefinitions::ByteOpCode::STACK_UP://Direct order of arguments
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = SpecialRegister::A0;
				operation.arg1 = command.arg0.data;
				result.push_back(operation);
				operation.code = VMOperationCode::ADD_RRR_UNSIGNED;
				operation.arg0 = SpecialRegister::SP;
				operation.arg1 = SpecialRegister::SP;
				operation.arg2 = SpecialRegister::A0;
				result.push_back(operation);
			}
				break;
			case LinkDefinitions::ByteOpCode::STACK_DOWN:
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = SpecialRegister::A0;
				operation.arg1 = command.arg0.data;
				result.push_back(operation);
				operation.code = VMOperationCode::SUB_RRR_UNSIGNED;
				operation.arg0 = SpecialRegister::SP;
				operation.arg1 = SpecialRegister::SP;
				operation.arg2 = SpecialRegister::A0;
				result.push_back(operation);
			}
				break;
			case LinkDefinitions::ByteOpCode::LEA_STATIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				result.push_back(operation);
				break;
			}	
			case LinkDefinitions::ByteOpCode::LEA_DYNAMIC:
				std::cerr << "Unsupported operation. Byte operation code:" << (uint8_t)command.code << "\n";
				return false;

			case LinkDefinitions::ByteOpCode::LOAD_DYNAMIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::LOAD_BY_ADDRESS;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				operation.arg2 = command.arg2.data;
				result.push_back(operation);
				break;
			}	
			case LinkDefinitions::ByteOpCode::STORE_DYNAMIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::STORE_BY_ADDRESS;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				operation.arg2 = command.arg2.data;
				result.push_back(operation);
				break;
			}
			case LinkDefinitions::ByteOpCode::LOAD_CONST_STATIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::LOAD_LOCAL;
				operation.arg0 = command.arg1.data;//REGISTER FOR SAVING
				operation.arg1 = command.arg0.data;//ADDRESS IN ROD
				operation.arg2 = command.arg2.data;//SIZE
				result.push_back(operation);
				break;
			}
			case LinkDefinitions::ByteOpCode::LEA_CONST:
			{
				std::cerr << "Unsupported operation. Byte operation code:" << (uint8_t)command.code << "\n";
				return false;
			}
			case LinkDefinitions::ByteOpCode::LOAD_STATIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::LOAD_LOCAL;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				operation.arg2 = command.arg2.data;
				result.push_back(operation);
				break;
			}
			case LinkDefinitions::ByteOpCode::STORE_STATIC:
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
			return true;
		}


		LinkDefinitions::ExecutionData TranslatorVM_1::Translate(fs::path directory, std::shared_ptr<LinkDefinitions::LinkingState> state, ObjectsReader& reader)
		{
			LinkDefinitions::ExecutionData execution_data;

			execution_data.read_only_data.allocate(state->constants_size);

			//moving data, constants freeing and offset calculating

			size_t rod_offset = 0;

			for (auto& constant: state->constants)
			{
				constant.memory_offset = rod_offset;

				if (constant.size_in_bytes + rod_offset > execution_data.read_only_data.bytes_size) 
				{
					std::cerr << "Invalid size calculating in the constants handling process.";
					execution_data.Free();
					state->FreeConstants();
					return LinkDefinitions::ExecutionData();
				}

				memcpy(execution_data.read_only_data.ptr + rod_offset, constant.data, constant.size_in_bytes); 
				rod_offset += constant.size_in_bytes;
				constant.Free();
			}
			rod_offset = ((rod_offset + DEFAULT_ALIGNMENT - 1) / DEFAULT_ALIGNMENT) * DEFAULT_ALIGNMENT;
			state->rod_offset_aligned = rod_offset;
			state->global_memory_offset = rod_offset;

			execution_data.aligned_rod_size = rod_offset;

			std::vector<VMOperation> commands;
			for (LinkDefinitions::moduleid id : state->linking_order) 
			{
				std::string file_name = state->module_prefix + std::to_string(id) + "." + state->module_extention;
				state->stack_offset_of_module[id] = state->global_memory_offset;
				bool check = HandleModule(directory / file_name, state, reader, commands,id);
				if (!check) 
				{
					std::cerr << "Error of module(" << id << ") handling. Module's name:" << file_name << "\n";
					execution_data.read_only_data.release();
					return LinkDefinitions::ExecutionData();
				}
			}

			execution_data.code.allocate(commands.size() * sizeof(VMOperation));

			::memcpy(execution_data.code.ptr,commands.data(), commands.size() * sizeof(VMOperation));

			return execution_data;
		}
		bool TranslatorVM_1::HandleModule(fs::path file, std::shared_ptr<LinkDefinitions::LinkingState> state, ObjectsReader& reader, std::vector<VMOperation>& commands, LinkDefinitions::moduleid id)
		{
			LinkDefinitions::static_bpointer  co_bytes = reader.ReadFile(file.string());
			if (co_bytes.ptr == nullptr)
			{
				std::cerr << "Invalid CO file \"" + file.string() + "\".\n";
				return false;
			}

			std::shared_ptr<LinkDefinitions::CommandsPool> pool = reader.DeserializeCO(co_bytes);

			pool->m_id = id;

			for (size_t i = 0; i < pool->commands.size(); i++) 
			{
				if (!HandleCommand(pool, state, commands, i)) return false;
			}

			state->global_memory_offset += pool->stack_size;
			return true;
		}
	}
}