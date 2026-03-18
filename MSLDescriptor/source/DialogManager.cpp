#include "..\include\DialogManager.hpp"


void DialogManager::DialogI()
{

	std::cout << "Enter path to file:";

	std::string path;
	std::cin.ignore();
	std::getline(std::cin, path);

	std::cout << "Attemp of loading started.\n";

	InterpretationState state = desc.LoadI(path);

	if (state.broken_file) 
	{
		return;
	}


	auto info = [](InterpretationState& state) {
		std::cout << "Size:" << state.file_size << " bytes.\n";
		std::cout << "Version:" << state.file_version << ".\n";
		std::cout << "Virtual machine code:" << (int)state.vm_type << ".\n"; 
		std::cout << "ROD offset(bytes):" << state.rod_offset << ".\n";
		std::cout << "Code offset(bytes):" << state.code_offset << ".\n";
		std::cout << "ROD size(bytes):" << state.rod_section.bytes_size << ".\n";
		std::cout << "Code size(bytes):" << state.code_section.bytes_size << ".\n";
		};

	std::cout << "File has been loaded.\n";

	info(state);

	while (true)
	{
		std::cout << "Select an action (exit - e):\n";

		std::cout << "1) Code view.\n";
		std::cout << "2) ROD view.\n";

		std::cout << ">";

		char choice;

		std::cin >> choice;

		

		if (choice == 'e') break;

		uint8_t number = choice - '0';

		switch (number)
		{
		case 1:
		{
			switch (state.vm_type)
			{
			case VMs::MSLVM_1:
			{
				MSLVM1::CodeChecker checker;

				if (!checker.IsBytesValid(state.code_section.bytes_size)) { std::cerr << "Invalid bytes size.\n"; break; }

				size_t index_size = checker.CalculateCodeSize(state.code_section.bytes_size);

				for (size_t i = 0; i < index_size; i++) 
				{
					checker.PrintIndex(state.code_section.ptr, i);
				}
				break;
			}
				
			default:
				std::cerr << "Invalid virtual machine.\n";
				break;
			}
		
		}
			break;
		default:
			std::cerr << "Invalid operation.\n";
			break;
		}
		
	}

	state.code_section.release();
	state.rod_section.release();

}

void DialogManager::Dialog()
{

	std::cout << "MSLDescriptor v0.1\n";

	std::unordered_map<uint8_t, std::pair<std::function<void()>, std::string>> actions =
	{
		{1, {[&]()->void {DialogI(); }, "View msli file"}}
	};

	while (true) 
	{
		std::cout << "Select an action (exit - e):\n";


		for (auto& t : actions) 
		{
			std::cout << std::to_string(t.first)  << ") " << t.second.second << ".\n";
		}

		std::cout << ">";

		char choice;

		std::cin >> choice;

		if (choice == 'e') break;

		uint8_t number = choice - '0';

		if (actions.count(number)) actions[number].first();
		else 
		{
			std::cerr << "Invalid operation.\n";
		}
	}
}
