#pragma once

namespace MSLC
{
	namespace CompilationInfo
	{
		namespace Functions
		{
			using FunctionID = size_t;
			constexpr FunctionID START_FUNCTION_ID = 1;
			constexpr FunctionID INVALID_FUNCTION_ID = 0;
		}
		namespace Types
		{
			using MSLTypeID = size_t;

			constexpr MSLTypeID START_TYPE_ID = 1;
			constexpr MSLTypeID INVALID_TYPE_ID = 0;
		}
	}
}