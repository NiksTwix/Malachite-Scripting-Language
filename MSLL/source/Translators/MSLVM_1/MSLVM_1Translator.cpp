#pragma once
#include "..\..\..\include\Translators\MSLVM_1\MSLVM_1Translator.hpp"


namespace MSLL
{
	namespace MSLVM_1
	{
		bool TranslatorVM_1::HandleCommand(std::shared_ptr<ObjectsInfo::CommandsPool> commands, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result, size_t& i)
		{

			ObjectsInfo::ByteCommand& cmd = commands->commands[i];

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
				default:
					break;
				}
			}

			return check;
		}

		bool TranslatorVM_1::HandleAL(ObjectsInfo::ByteCommand command, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result)
		{
			return false;
		}

		bool TranslatorVM_1::HandleCF(ObjectsInfo::ByteCommand command, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result)
		{
			return false;
		}

		bool TranslatorVM_1::HandleMR(ObjectsInfo::ByteCommand command, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result)
		{
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
			std::vector<VMOperation> commands;
			for (ObjectsInfo::moduleid id : state->linking_order) 
			{
				std::string file_name = state->module_prefix + std::to_string(id) + "." + state->module_extention;
				HandleModule(directory / file_name, state, reader, commands);
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

			bool check = true;

			for (size_t i = 0; i < pool->commands.size(); i++) 
			{
				check = check && HandleCommand(pool, state, commands, i);
			}


			return check;
		}
	}
}