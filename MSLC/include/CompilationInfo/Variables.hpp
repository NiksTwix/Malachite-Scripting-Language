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
				uint64_t local_stack_offset;	//In the byte code
				uint64_t global_stack_offset;	//In the byte code
				Values::ValueInfo vinfo;
				Definitions::AccessMode access_mode = Definitions::AccessMode::Public;
			};	
		}
	}
}