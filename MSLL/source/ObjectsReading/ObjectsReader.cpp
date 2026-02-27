#include "..\..\include\ObjectsReading\ObjectsReader.hpp"
#include <fstream>
#include <iostream>
namespace MSLL
{
	namespace MD = ObjectsInfo::MSLOData;
	void ObjectsReader::HandleLOSection(std::pair<char*, size_t> bytes_buffer, std::shared_ptr<ObjectsInfo::LinkingState> state, size_t section_offset, size_t section_size)
	{
		size_t offset = section_offset;

		while (offset < section_offset + section_size)
		{
			ObjectsInfo::moduleid m_id;
			memcpy(&m_id, bytes_buffer.first + offset, sizeof(ObjectsInfo::moduleid));
			state->linking_order.push_back(m_id);
			offset += sizeof(ObjectsInfo::moduleid);
		}

	}
	void ObjectsReader::HandleConstantsSection(std::pair<char*, size_t> bytes_buffer, std::shared_ptr<ObjectsInfo::LinkingState> state, size_t section_offset, size_t section_size)
	{
		size_t offset = section_offset;
		while (offset < section_offset + section_size)
		{
			size_t c_id;
			memcpy(&c_id, bytes_buffer.first + offset, sizeof(size_t));
			offset += sizeof(size_t);
			size_t data_size;
			memcpy(&data_size, bytes_buffer.first + offset, sizeof(size_t));
			offset += sizeof(size_t);
			ObjectsInfo::ConstantData constant;
			constant.InitBy(bytes_buffer.first + offset, data_size);
			offset += data_size;
			constant.id = c_id;
		}
	}
	void ObjectsReader::HandleSymbolSection(std::pair<char*, size_t> bytes_buffer, std::shared_ptr<ObjectsInfo::LinkingState> state, size_t section_offset, size_t section_size)
	{
		size_t offset = section_offset;
		while (offset < section_offset + section_size)
		{
			size_t s_id;
			memcpy(&s_id, bytes_buffer.first + offset, sizeof(size_t));
			offset += sizeof(size_t);
			size_t offset_in_code_base;
			memcpy(&offset_in_code_base, bytes_buffer.first + offset, sizeof(size_t));
			offset += sizeof(size_t);
			ObjectsInfo::moduleid module_;
			memcpy(&module_, bytes_buffer.first + offset, sizeof(ObjectsInfo::moduleid));
			offset += sizeof(ObjectsInfo::moduleid);
			ObjectsInfo::SymbolType stype = ObjectsInfo::SymbolType::Undefined;
			memcpy(&stype, bytes_buffer.first + offset, sizeof(ObjectsInfo::SymbolType));
			offset += sizeof(ObjectsInfo::SymbolType);


			state->symbols.push_back(ObjectsInfo::SymbolData(s_id, offset_in_code_base, module_, stype));
		}
	}
	std::shared_ptr<ObjectsInfo::LinkingState> ObjectsReader::DeserializeMO(std::pair<char*, size_t> bytes_buffer)
	{
		if (bytes_buffer.first == nullptr || bytes_buffer.second == 0) return nullptr;

		if (bytes_buffer.second < MD::mo_header_reserved_size)
		{
			std::cerr << "Invalid main object file.\n";
			return nullptr;
		}

		std::shared_ptr<ObjectsInfo::LinkingState> linking_state = std::make_shared<ObjectsInfo::LinkingState>();


		bool magic_check = memcmp(bytes_buffer.first, MD::mo_magic.data(), MD::magic_size);

		if (magic_check)
		{
			std::cerr << "Attemp to deserialize not MO file.\n";
			return nullptr;
		}

		size_t offset = MD::magic_size;

		memcpy(&linking_state->version, bytes_buffer.first + offset, sizeof(linking_state->version));
		offset += sizeof(linking_state->version);

		if (linking_state->version != MD::FilesVersion)
		{
			std::clog << "Versions are different. Errors can occur during the reading process.\n";
		}

		memcpy(&linking_state->compilation_flags, bytes_buffer.first + offset, sizeof(linking_state->compilation_flags));
		offset += sizeof(linking_state->compilation_flags);

		char module_prefix[MD::size_of_module_prefix] = {};
		char module_extension[MD::size_of_module_extension] = {};

		memcpy(module_prefix, bytes_buffer.first + offset, MD::size_of_module_prefix);
		offset += MD::size_of_module_prefix;
		memcpy(module_extension, bytes_buffer.first + offset, MD::size_of_module_extension);
		offset += MD::size_of_module_extension;
		linking_state->module_extention = std::string(module_extension,MD::size_of_module_extension);
		linking_state->module_prefix = std::string(module_prefix, MD::size_of_module_prefix);

		//Sections reading

		for (size_t i = 0; i < MD::section_count; i++) 
		{
			ObjectsInfo::SectionType section_type = static_cast<ObjectsInfo::SectionType>(*(bytes_buffer.first + offset));
			offset += sizeof(ObjectsInfo::SectionType);
			size_t section_offset;
			memcpy(&section_offset, bytes_buffer.first + offset, sizeof(size_t));
			offset += sizeof(size_t);
			size_t section_size;
			memcpy(&section_size, bytes_buffer.first + offset, sizeof(size_t));
			offset += sizeof(size_t);

			if (offset + section_size > bytes_buffer.second) 
			{
				std::cerr << "Invalid section data: offet + size of section is greater than size of file. Offset: " << std::hex << offset - 2 *sizeof(size_t) - sizeof(ObjectsInfo::SectionType) << "\n";
				return nullptr;
			}

			switch (section_type)
			{
			case MSLL::ObjectsInfo::LINKING_ORDER:
				HandleLOSection(bytes_buffer, linking_state, section_offset, section_size);
				break;
			case MSLL::ObjectsInfo::CONSTANTS:
				HandleConstantsSection(bytes_buffer, linking_state, section_offset, section_size);
				break;
			case MSLL::ObjectsInfo::SYMBOLS:
				HandleSymbolSection(bytes_buffer, linking_state, section_offset, section_size);
				break;
			case MSLL::ObjectsInfo::DEBUG:
				//In the future
				break;
			default:
				break;
			}
		}
		

		return linking_state;
	}
	std::shared_ptr<ObjectsInfo::CommandsPool> ObjectsReader::DeserializeCO(std::pair<char*, size_t> bytes_buffer)
	{
		if (bytes_buffer.first == nullptr || bytes_buffer.second == 0) return nullptr;

		if (bytes_buffer.second < MD::co_header_reserved_size)
		{
			std::cerr << "Invalid code object file.\n";
			return nullptr;
		}

		std::shared_ptr<ObjectsInfo::CommandsPool> result = std::make_shared<ObjectsInfo::CommandsPool>();

		bool magic_check = memcmp(bytes_buffer.first, MD::co_magic.data(), MD::magic_size);

		if (magic_check)
		{
			std::cerr << "Attemp to deserialize not CO file.\n";
			return nullptr;
		}
		size_t offset = MD::magic_size;

		memcpy(&result->version, bytes_buffer.first + offset, sizeof(result->version));
		offset += sizeof(result->version);

		if (result->version != MD::FilesVersion)
		{
			std::clog << "Versions are different. Errors can occur during the reading process.\n";
		}
		memcpy(&result->compilation_flags, bytes_buffer.first + offset, sizeof(result->compilation_flags));

		offset += sizeof(result->compilation_flags);


		memcpy(&result->code_size_in_bytes, bytes_buffer.first + offset, sizeof(result->code_size_in_bytes));
		offset += sizeof(result->code_size_in_bytes);

		offset = MD::co_header_reserved_size;
		//Commands
		while (offset < MD::co_header_reserved_size + result->code_size_in_bytes) 
		{
			ObjectsInfo::ByteOpCode opcode;
			memcpy(&opcode, bytes_buffer.first + offset, sizeof(opcode)); offset += sizeof(opcode);
			ObjectsInfo::CommandSource source0;
			memcpy(&source0, bytes_buffer.first + offset, sizeof(source0)); offset += sizeof(source0);
			size_t arg0;
			memcpy(&arg0, bytes_buffer.first + offset, sizeof(arg0)); offset += sizeof(arg0);
			ObjectsInfo::CommandSource source1;
			memcpy(&source1, bytes_buffer.first + offset, sizeof(source1)); offset += sizeof(source1);
			size_t arg1;
			memcpy(&arg1, bytes_buffer.first + offset, sizeof(arg1)); offset += sizeof(arg1);
			ObjectsInfo::CommandSource source2;
			memcpy(&source2, bytes_buffer.first + offset, sizeof(source2)); offset += sizeof(source2);
			size_t arg2;
			memcpy(&arg2, bytes_buffer.first + offset, sizeof(arg2)); offset += sizeof(arg2);
			uint32_t flags;
			memcpy(&flags, bytes_buffer.first + offset, sizeof(flags)); offset += sizeof(flags);
			uint32_t source_line;
			memcpy(&flags, bytes_buffer.first + offset, sizeof(source_line)); offset += sizeof(source_line);
			ObjectsInfo::ByteCommand command(opcode, ObjectsInfo::CommandArgument(arg0, source0), ObjectsInfo::CommandArgument(arg1, source1), ObjectsInfo::CommandArgument(arg2, source2), flags, source_line);
			result->commands.push_back(command);
		}


		return result;
	}
	std::pair<char*, size_t> ObjectsReader::ReadFile(const std::string& path)
	{
		std::ifstream  file(path, std::ios::binary);

		if (!file) 
		{
			std::cerr << "Read file error. Path:" << path << "\n";
			return {nullptr,0 };
		}

		file.seekg(0, std::ios::end);
		std::streamsize size = file.tellg();		//file.tellg returns position of file pointer
		file.seekg(0, std::ios::beg);

		char* allocated = static_cast<char*>(calloc(size, 1));

		file.read(allocated, size);

		return { allocated, size};
	}
}