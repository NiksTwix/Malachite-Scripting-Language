#pragma once
#include "..\ObjectsReading\LinkDefinitions.hpp"

#include "filesystem"

namespace fs = std::filesystem;




namespace MSLL
{
	namespace MSLI
	{
		constexpr std::string_view magic = "MSLI";

		constexpr float version = 1.0f;


		constexpr size_t header_size = 100;
	}



	class ECWriter {
	public:

		bool SaveAsMSLI(fs::path directory, fs::path name, LinkDefinitions::ExecutionData& data, bool free_ed = false);

	};
}