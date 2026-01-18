#pragma once
#include "CommonInfo.hpp"
#include <unordered_map>

namespace MSLC 
{
	namespace CompilationInfo
	{
		namespace Variables
		{
			struct VariableDescription
			{
				std::string name;
				VariableID id;
				Values::ValueInfo vinfo;
				Definitions::AccessMode access_mode = Definitions::AccessMode::Public;
			};	
		}
	}
}