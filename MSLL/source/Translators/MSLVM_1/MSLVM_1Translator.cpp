#pragma once
#include "..\..\..\include\Translators\MSLVM_1\MSLVM_1Translator.hpp"


namespace MSLL
{
	namespace MSLVM_1
	{
		ObjectsInfo::ExecutionData TranslatorVM_1::Translate(fs::path directory, std::shared_ptr<ObjectsInfo::LinkingState> state, ObjectsReader& reader)
		{
			ObjectsInfo::ExecutionData execution_data;

			execution_data.read_only_data.first = static_cast<char*>(calloc(state->constants_size, 1));
			execution_data.read_only_data.second = state->constants_size;

			//moving data, constants freeing and offset calculating

			size_t c_offset = 0;

			for (auto& constant: state->constants)
			{
				constant.memory_offset = c_offset;

				if (constant.size_in_bytes + c_offset > execution_data.read_only_data.second) 
				{
					std::cerr << "Invalid size calculating in the constants handling process.";
					execution_data.Free();
					state->FreeConstants();
					return ObjectsInfo::ExecutionData();
				}

				memcpy(execution_data.read_only_data.first + c_offset, constant.data, constant.size_in_bytes); 
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
			return true;
		}
	}
}