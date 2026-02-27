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
			std::shared_ptr<ObjectsInfo::ExecutionData> Translate(fs::path directory, std::shared_ptr<ObjectsInfo::LinkingState> state, ObjectsReader& reader);
		};

	}
}