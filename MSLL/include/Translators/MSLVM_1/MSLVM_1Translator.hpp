#pragma once

#include "MSLVM_1Common.hpp"
#include "..\..\ObjectsReading\ObjectsReader.hpp"
#include "filesystem"

namespace fs = std::filesystem;

namespace MSLL
{
	namespace MSLVM_1
	{

		class TranslatorVM_1
		{
		private:

			bool HandleCommand(std::shared_ptr<ObjectsInfo::CommandsPool> commands, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result, size_t& i);

			bool HandleAL(ObjectsInfo::ByteCommand command, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result);
			bool HandleCF(ObjectsInfo::ByteCommand command, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result);
			bool HandleMR(ObjectsInfo::ByteCommand command, std::shared_ptr<ObjectsInfo::LinkingState> state, std::vector<VMOperation>& result);
		public:
			ObjectsInfo::ExecutionData Translate(fs::path directory, std::shared_ptr<ObjectsInfo::LinkingState> state, ObjectsReader& reader);

			bool HandleModule(fs::path file, std::shared_ptr<ObjectsInfo::LinkingState> state, ObjectsReader& reader, std::vector<VMOperation>& commands, ObjectsInfo::moduleid id);

		};

	}
}