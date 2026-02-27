#pragma once
#include "ObjectsReading/ObjectsReader.hpp"

namespace MSLL
{
	enum class VMs: uint8_t
	{
		MSLVM_1,
	};


	class Linker 
	{
	private:
		std::vector<std::string> vm_list = { "mslvm_1" };

		ObjectsReader reader;

		bool output_mode = 0;

	public:
		const std::vector<std::string>& GetVMList() const { return vm_list; }


		bool Link(std::string& path, VMs vm_index);

		void SetOutputMode(bool new_output_mode) { output_mode = new_output_mode; }

	};
}