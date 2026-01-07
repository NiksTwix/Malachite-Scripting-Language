#pragma once

namespace MSLC
{
	namespace CompilationInfo
	{

		using DescriptionID = size_t;
		constexpr DescriptionID INVALID_ID = SIZE_MAX;

		constexpr DescriptionID START_COUNTER_ID = 0;

		namespace Functions
		{
			using FunctionID = DescriptionID;
		}
		namespace Types
		{
			using TypeID = DescriptionID;
		}

		namespace Variables
		{
			using VariableID = DescriptionID;
		}
	}
}