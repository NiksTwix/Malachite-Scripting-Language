#pragma once

#include "MSLVM_1Common.hpp"
#include "filesystem"

namespace fs = std::filesystem;

namespace MSLL
{
	namespace MSLVM_1
	{

		class TranslatorVM_1
		{
		private:

			bool HandleCommand(std::shared_ptr<LinkDefinitions::CommandsPool> commands, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result, size_t& i);

			bool HandleAL(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result);	//ariithmetic and logic
			bool HandleCF(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result);	//control flow
			bool HandleMR(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result);	//memory

			bool HandleSP(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result);	//SPECIAL


			void PushVMCommand(VMOperationCode code, LinkDefinitions::CommandArgument arg0, LinkDefinitions::CommandArgument arg1, LinkDefinitions::CommandArgument arg2, std::vector<VMOperation>& result);

		public:
			LinkDefinitions::ExecutionData Translate(fs::path directory, std::shared_ptr<LinkDefinitions::LinkingState> state, ObjectsReader& reader);

			bool HandleModule(fs::path file, std::shared_ptr<LinkDefinitions::LinkingState> state, ObjectsReader& reader, std::vector<VMOperation>& commands, LinkDefinitions::moduleid id);

		};

	}
}