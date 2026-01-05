#include "..\..\include\CompilationInfo\CompilationState.hpp"

namespace MSLC
{
	namespace CompilationInfo
	{
		Preprocessing::MacrosTable& CompilationState::GetMacrosTable()
		{
			return macros_table;
		}
	}
}