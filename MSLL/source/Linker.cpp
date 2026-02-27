#include "..\include\Linker.hpp"
#include "..\include\Translators\MSLVM_1\MSLVM_1Translator.hpp"



namespace MSLL
{
	bool Linker::Link(std::string& path, VMs vm_index)
	{
		//MO file loading

		std::pair<char*, size_t> mo_bytes = reader.ReadFile(path);
		if (mo_bytes.first == nullptr) 
		{
			std::cerr << "Invalid MO file.\n";
			return false;
		}

		fs::path directory_path = fs::path(path).parent_path();

		auto state = reader.DeserializeMO(mo_bytes);

		switch (vm_index)
		{
		case MSLL::VMs::MSLVM_1:
		{
			MSLVM_1::TranslatorVM_1 translator;
			auto execution_data = translator.Translate(directory_path, state, reader);

			// write to msl exe format file
		}
			break;
		default:
			break;
		}


		return true;
	}
}