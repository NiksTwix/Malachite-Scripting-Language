#pragma once
#include "vm_types.hpp"
#include "vm_execution.hpp"


namespace MSLVM 
{
	class VirtualMachine 
	{
	public:

		std::vector<VMOperation> LoadBinary(const std::string& path);

	private:
		VMState state;
	};
}