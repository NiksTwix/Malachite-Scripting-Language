#include "..\include\DialogManager.hpp"


void DialogManager::PrintDump(InterpretationState& state)
{
	size_t address = 0;

	while (address < state.rod_section.bytes_size)
	{
		std::cout << std::hex << address << "|";

		size_t difference = state.rod_section.bytes_size - address > 8
			? 8
			: state.rod_section.bytes_size - address;

		size_t i = 0;
		while (i < difference)
		{
			switch (rv_type)
			{
			case RODViewType::HEX_BYTES:
			{
				uint8_t val;
				memcpy(&val, state.rod_section.ptr + address + i, 1);
				std::cout << std::hex << (int)val;
				i += 1;
				break;
			}
			case RODViewType::DEC_BYTES:
			{
				int8_t val;
				memcpy(&val, state.rod_section.ptr + address + i, 1);
				std::cout << std::dec << (int)val;
				i += 1;
				break;
			}
			case RODViewType::HEX_WORD:
			{
				if (i + 2 > difference) goto break_loop;
				uint16_t val;
				memcpy(&val, state.rod_section.ptr + address + i, 2);
				std::cout << std::hex << val;
				i += 2;
				break;
			}
			case RODViewType::DEC_WORD:
			{
				if (i + 2 > difference) goto break_loop;
				uint16_t val;
				memcpy(&val, state.rod_section.ptr + address + i, 2);
				std::cout << std::dec << val;
				i += 2;
				break;
			}
			case RODViewType::HEX_DWORD:
			{
				if (i + 4 > difference) goto break_loop;
				uint32_t val;
				memcpy(&val, state.rod_section.ptr + address + i, 4);
				std::cout << std::hex << val;
				i += 4;
				break;
			}
			case RODViewType::DEC_DWORD:
			{
				if (i + 4 > difference) goto break_loop;
				uint32_t val;
				memcpy(&val, state.rod_section.ptr + address + i, 4);
				std::cout << std::dec << val;
				i += 4;
				break;
			}
			case RODViewType::HEX_QWORD:
			{
				if (i + 8 > difference) goto break_loop;
				uint64_t val;
				memcpy(&val, state.rod_section.ptr + address + i, 8);
				std::cout << std::hex << val;
				i += 8;
				break;
			}
			case RODViewType::DEC_QWORD:
			{
				if (i + 8 > difference) goto break_loop;
				uint64_t val;
				memcpy(&val, state.rod_section.ptr + address + i, 8);
				std::cout << std::dec << val;
				i += 8;
				break;
			}
			default:
				i = difference; // exit loop
				break;
			}
			std::cout << " ";
		}
	break_loop:
		address += i;
		std::cout << "\n";
	}
}

void DialogManager::DialogROD(InterpretationState& state)
{
	while (true) 
	{
		std::cout << "Select an action (exit - e):\n";

		std::cout << "1) Switch view mode.\n";
		std::cout << "2) Dump's view.\n";
		std::cout << ">";

		char choice;

		std::cin >> choice;



		if (choice == 'e') break;

		uint8_t number = choice - '0';

		switch (number)
		{
		case 1:
		{
			for (size_t i = 0; i < rv_types.size(); i++) 
			{
				std::cout << i + 1 << ")" << rv_types_string[(RODViewType)i] << "\n";
			
			}
			std::cout << "Current: " << rv_types_string[rv_type] << "\n";

			std::cout << ">";

			char choice;

			std::cin >> choice;

			uint8_t uint_choice = choice - '0' - 1;

			if (uint_choice > (uint8_t)RODViewType::DEC_QWORD) 
			{
				std::cerr << "Invalid type.\n";
				break;
			}
			rv_type = (RODViewType)uint_choice;
		}
		break;

		case 2:
		{
			PrintDump(state);
			break;
		}
		default:
			std::cerr << "Invalid operation.\n";
			break;
		}
	}
}



void DialogManager::PrintCode(InterpretationState& state)
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

void DialogManager::PrintVMInfo(InterpretationState& state)
{
	switch (state.vm_type)
	{
	case VMs::MSLVM_1:
	{
		MSLVM1::CodeChecker checker;

		checker.PrintVMInfo();
		break;
	}

	default:
		std::cerr << "Invalid virtual machine.\n";
		break;
	}
}

void DialogManager::DialogI()
{

	std::cout << "Enter path to file:";

	std::string path;
	std::cin.ignore();
	std::getline(std::cin, path);

	std::cout << "Attemp of loading started.\n";

	if (path.empty())
	{
		std::cout << "Invalid path.";
		return;
	}
	else if (path.front() == '"' && path.back() == '"')
	{
		path = path.substr(1, path.size() - 2);
	}


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
		std::cout << "3) Reload.\n";
		std::cout << "4) VM info.\n";
		std::cout << ">";

		char choice;

		std::cin >> choice;

		

		if (choice == 'e') break;

		uint8_t number = choice - '0';

		switch (number)
		{
		case 1:
		{
			PrintCode(state);
		}
			break;

		case 2: 
		{
			DialogROD(state);
			break;
		}
		case 3:
		{
			state.Reset();	//Clear resources;
			state = desc.LoadI(path);

			if (state.broken_file)
			{
				std::cerr << "File is broken.\n";
				return;
			}
			std::cout << "Reloaded.\n";

			info(state);

			continue;
		}
		case 4:
		{
			PrintVMInfo(state);
			continue;
		}
		default:
			std::cerr << "Invalid operation.\n";
			break;
		}
		
	}

	state.Reset();

}

void DialogManager::Dialog()
{

	std::cout << "MSLDescriptor v0.2\n";

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
