#include "..\..\include\Packing\Packer.hpp"


namespace MSLC
{
	namespace Packing
	{
		std::pair<char*, size_t> Packer::ByterizeCompilationState(CompilationInfo::CompilationState* cs_state, Preprocessing::HandlingOrder& ho)
		{
			//Calc size of array for allocation
			size_t size_of_data = MSLOSizes::mo_header_reserved_size;

			size_t linking_order_offset = size_of_data;
			
			size_of_data += ho.processing_order.size() * MSLOSizes::size_of_module_id;	//2 bytes on number
			size_t linking_order_size = size_of_data - linking_order_offset;
			size_t constants_offset = size_of_data;

			for (size_t i = 0; i < cs_state->GetICT().constants_by_id.size(); i++) 
			{
				size_of_data += MSLOSizes::size_of_constant_field_header + cs_state->GetICT().constants_by_id[i].GetDataSize();
			}
			size_t constants_size = size_of_data - constants_offset;
			size_t symbols_offset = size_of_data;
			for (size_t i = 0; i < cs_state->GetUnhandledSymbolsCount(); i++)
			{
				size_of_data += MSLOSizes::size_of_symbol_field;
			}
			size_t symbols_size = size_of_data - symbols_offset;
			size_t debug_offset = size_of_data;

			/*
				Debug info in the future
			*/
			size_of_data += 100;	//just like that

			size_t debug_size = size_of_data - debug_offset;

			size_t current_offset = 0;

			char* allocated = static_cast<char*>(calloc(size_of_data, 1));

			//Filling
			memcpy(allocated, "MSLMO", 5);	//Magic

			current_offset += 5;

			WriteBytes(allocated + current_offset, &FilesVersion, sizeof(FilesVersion), current_offset);	//Version
			WriteBytes(allocated + current_offset, 0, sizeof(uint16_t), current_offset);	//Flags

			WriteBytes(allocated + current_offset, module_prefix,  MSLOSizes::module_file_prefix, current_offset);	//Module prefix
			WriteBytes(allocated + current_offset, module_extension, MSLOSizes::module_extension, current_offset);	//Module extension

			//Sections
			WriteSectionInfo(allocated, current_offset, SectionType::LINKING_ORDER, linking_order_offset, linking_order_size);
			WriteSectionInfo(allocated, current_offset, SectionType::CONSTANTS, constants_offset, constants_size);
			WriteSectionInfo(allocated, current_offset, SectionType::SYMBOLS, symbols_offset, symbols_size);
			WriteSectionInfo(allocated, current_offset, SectionType::DEBUG, debug_offset, debug_size);

			//Header ended
			current_offset = MSLOSizes::mo_header_reserved_size;
			//Linking order
			//Linking order bytes
			for (size_t id : ho.processing_order) {
				uint16_t id16 = static_cast<uint16_t>(id);
				WriteBytes(allocated + current_offset, &id16, sizeof(id16), current_offset);
			}
			//Constants
			for (size_t i = 0; i < cs_state->GetICT().constants_by_id.size(); i++)
			{
				WriteBytes(allocated + current_offset, &i, MSLOSizes::size_of_id, current_offset);	//ID
				auto data_size = cs_state->GetICT().constants_by_id[i].GetDataSize();
				WriteBytes(allocated + current_offset, &data_size, MSLOSizes::size_of_size, current_offset);	//Size
				WriteBytes(allocated + current_offset, cs_state->GetICT().constants_by_id[i].GetBytes(), data_size, current_offset);	//Data
			}
			//Symbols
			for (size_t i = 0; i < cs_state->GetUnhandledSymbolsCount(); i++)
			{
				WriteBytes(allocated + current_offset, &i, MSLOSizes::size_of_id, current_offset);	//ID
				auto us = cs_state->GetUnhandledSymbol(i);

				switch (us->symbol_type)
				{
					case CompilationInfo::SymbolType::Function:
						WriteBytes(allocated + current_offset, &cs_state->GetGST().GetFunction(us->desc_id)->start_byte_ip, MSLOSizes::size_of_size, current_offset);	//Offset in the code base of mslco
						WriteBytes(allocated + current_offset, &cs_state->GetGST().GetFunction(us->desc_id)->module_id, MSLOSizes::size_of_module_id, current_offset);	//Module id
						break;
					case CompilationInfo::SymbolType::Variable:
						WriteBytes(allocated + current_offset, &cs_state->GetGST().GetVariable(us->desc_id)->global_stack_offset, MSLOSizes::size_of_size, current_offset);	//Offset in the memory
						WriteBytes(allocated + current_offset, &cs_state->GetGST().GetVariable(us->desc_id)->module_id, MSLOSizes::size_of_module_id, current_offset);	//Module id
						break;
				default:
					break;
				}
				WriteBytes(allocated + current_offset, &us->symbol_type, sizeof(CompilationInfo::SymbolType), current_offset);	//SymbolType
				
			}

			//Debug
			//In the future
			return { allocated, size_of_data };
		}
		std::pair<char*, size_t> Packer::ByterizeCommands(CompilationInfo::CompilationState* cs_state, BCommandsArray& commands, size_t module_id)
		{
			size_t size_of_data = MSLOSizes::co_header_reserved_size;

			size_t command_offset = size_of_data;
			size_t commands_size = commands.Size() * MSLOSizes::command_size;
			size_of_data += commands.Size() * MSLOSizes::command_size;

			size_t current_offset = 0;

			char* allocated = static_cast<char*>(calloc(size_of_data, 1));

			memcpy(allocated, "MSLCO", 5);	//Magic

			current_offset += 5;

			WriteBytes(allocated + current_offset, &FilesVersion, sizeof(FilesVersion), current_offset);	//Version
			WriteBytes(allocated + current_offset, 0, sizeof(uint16_t), current_offset);	//Flags

			WriteBytes(allocated + current_offset, &commands_size, sizeof(size_t), current_offset);	//Code size

			for (size_t i = 0; i < commands.Size(); i++) 
			{
				ByteCommand& command = commands[i];

				WriteBytes(allocated + current_offset, &command.code, sizeof(ByteOpCode), current_offset);	//OpCode
				WriteBytes(allocated + current_offset, &command.arg0.type, sizeof(command.arg0.type), current_offset);	//Type of arg0
				WriteBytes(allocated + current_offset, &command.arg0.data, sizeof(command.arg0.data), current_offset);	//Data of arg0
				WriteBytes(allocated + current_offset, &command.arg1.type, sizeof(command.arg1.type), current_offset);	//Type of arg1
				WriteBytes(allocated + current_offset, &command.arg1.data, sizeof(command.arg1.data), current_offset);	//Data of arg1
				WriteBytes(allocated + current_offset, &command.arg2.type, sizeof(command.arg2.type), current_offset);	//Type of arg2
				WriteBytes(allocated + current_offset, &command.arg2.data, sizeof(command.arg2.data), current_offset);	//Data of arg2
				WriteBytes(allocated + current_offset, &command.flags, sizeof(command.flags), current_offset);	//Flags
			}


			return { allocated,size_of_data};
		}
	}
}