#pragma once
#include "ObjectsReading/ObjectsReader.hpp"
namespace MSLL
{
	class Linker 
	{
	private:
		std::vector<std::string> vm_list = { "mslvm_1" };
	public:
		const std::vector<std::string>& GetVMList() const { return vm_list; }


		bool Link(std::string& path, uint16_t vm_index);

	};
}