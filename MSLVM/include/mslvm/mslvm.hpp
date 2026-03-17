#pragma once
#include "vm_types.hpp"
#include "vm_execution.hpp"


namespace MSLVM 
{
	class VirtualMachine 
	{
	private:
		VMState state;
	public:

		bool LoadScript(const std::string& path);

		VMState& GetState() { return state; }

	
	};
}