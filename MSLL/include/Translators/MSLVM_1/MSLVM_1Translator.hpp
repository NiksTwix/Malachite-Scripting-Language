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
		public:
			ObjectsInfo::ExecutionData Translate(fs::path directory, std::shared_ptr<ObjectsInfo::LinkingState> state, ObjectsReader& reader);

			bool HandleModule(fs::path file, std::shared_ptr<ObjectsInfo::LinkingState> state, ObjectsReader& reader, std::vector<VMOperation>& commands);

		};

	}
}