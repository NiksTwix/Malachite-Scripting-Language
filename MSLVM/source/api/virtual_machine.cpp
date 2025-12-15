#include "..\..\include\mslvm\mslvm.hpp"
#include <fstream>


namespace MSLVM 
{
	std::vector<VMOperation> VirtualMachine::LoadBinary(const std::string& path)
	{
		// Loading
		
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			return std::vector<VMOperation>();
		}
		file.seekg(0, std::ios::end);
		size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		size_t numCommands = fileSize / sizeof(VMOperation);
		std::vector<VMOperation> commands(numCommands);

		// READ ALL AS STRUCTURE'S ARRAY!
		file.read(reinterpret_cast<char*>(commands.data()), fileSize);

		return commands;
	}
}