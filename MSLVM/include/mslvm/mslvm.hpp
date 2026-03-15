#pragma once
#include "vm_types.hpp"
#include "vm_execution.hpp"


namespace MSLVM 
{
	class VirtualMachine 
	{
	public:

		bool LoadScript(const std::string& path);

	private:
		VMState state;
	};
}