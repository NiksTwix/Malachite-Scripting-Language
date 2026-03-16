#pragma once
#include "..\ObjectsReading\ObjectsInfo.hpp"

#include "filesystem"

namespace fs = std::filesystem;



namespace MSLL
{
	class ECWriter {
	private:

	public:

		bool SaveAsMSLI(fs::path& directory, fs::path& name, ObjectsInfo::ExecutionData& data, bool free_ed = false);

	};
}