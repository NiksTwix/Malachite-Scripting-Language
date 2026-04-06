#pragma once
#include <cstdint>



namespace MSLC 
{
	namespace Definitions 
	{
		constexpr size_t NULLPTR_VALUE = SIZE_MAX;
		constexpr size_t NULL_VALUE = 0;
		constexpr bool FALSE_VALUE = false;
		constexpr bool TRUE_VALUE = true;

		using ModuleId = uint16_t;
	}

}