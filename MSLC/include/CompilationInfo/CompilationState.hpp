#pragma once
#include "..\Preprocessing\MacrosInfo.hpp"


namespace MSLC 
{
	namespace CompilationInfo
	{

		class CompilationState 
		{
			Preprocessing::MacrosTable macros_table;

		public:
			Preprocessing::MacrosTable& GetMacrosTable();
		};


	}
}