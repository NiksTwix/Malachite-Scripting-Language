#pragma once
#include "..\Definitions\ValueContainer.hpp"
#include "..\Definitions\CommonEnums.hpp"
#include "CommonInfo.hpp"


namespace MSLC 
{
	namespace CompilationInfo
	{
		namespace Variables
		{

			enum VariableFlags
			{
				None = 0,
				Const = 1 << 0,
			};


			struct VariableDescription
			{
				std::string name;

				VariableID id;

				Types::TypeID type_id;

				size_t stack_offset = 0;

				Definitions::AccessMode access_mode = Definitions::AccessMode::Public;	//in namespaces 
			};


		}
	}
}