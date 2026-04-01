#pragma once
#include <cstdint>



namespace MSLC 
{
	namespace Definitions 
	{
		enum class AccessMode 
		{
			Public,
			Protected,
			Private
		};

		using moduleid = uint16_t;

		constexpr size_t NULLPTR_VALUE = SIZE_MAX;
		constexpr size_t NULL_VALUE = 0;
		constexpr bool FALSE_VALUE = false;
		constexpr bool TRUE_VALUE = true;
	}

}