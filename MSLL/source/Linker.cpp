#include "..\include\Linker.hpp"
#include "..\include\Translators\MSLVM_1\MSLVM_1Translator.hpp"



namespace MSLL
{
	bool Linker::Link(std::string& path, LinkDefinitions::VMs vm_index)
	{
		//MO file loading

		LinkDefinitions::static_bpointer  mo_bytes = reader.ReadFile(path);
		if (!mo_bytes.is_valid())
		{
			std::cerr << "Invalid MO file.\n";
			return false;
		}

		fs::path directory_path = fs::path(path).parent_path();

		fs::path name = fs::path(path).stem();

		auto state = reader.DeserializeMO(mo_bytes);

		mo_bytes.release();

		switch (vm_index)
		{
		case LinkDefinitions::VMs::MSLVM_1:
		{
			MSLVM_1::TranslatorVM_1 translator;
			auto execution_data = translator.Translate(directory_path, state, reader);

			execution_data.vm_type = LinkDefinitions::VMs::MSLVM_1;

			bool result = ec_writer.SaveAsMSLI(directory_path, name.string() + ".msli", execution_data, true);

			if (output_mode) 
			{
				if (!result) std::cerr << "File saving has failed.\n";
				else 
				{
					std::cout << "File has been successfully saved on path:\"" << (directory_path / name).string() + ".msli\".\n";
					return result;
				}
			}
		}
			break;
		default:
			std::cerr << "Invalid vm index.\n";
			break;
		}


		return false;
	}
}