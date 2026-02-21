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

	}

}