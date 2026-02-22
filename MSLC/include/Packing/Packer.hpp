#pragma once
#include "..\IR\ByteTranslation\ByteTranslationInfo.hpp"
#include "..\Preprocessing\AMGBuilder.hpp"
#include <string_view>
#include <filesystem>

namespace fs = std::filesystem;
namespace MSLC 
{
	namespace Packing 
	{
		using namespace IntermediateRepresentation::Byte;
		
	    
	    
		namespace MSLOData		//in bytes
		{
			constexpr uint32_t mo_header_reserved_size = 500;
			constexpr uint32_t co_header_reserved_size = 100;

			constexpr uint32_t magic_size = 5;
			constexpr uint32_t size_of_module_prefix = 32;//31 character + \0
			constexpr uint32_t size_of_module_extension = 10;
			constexpr uint32_t section_count = 4;

			constexpr uint32_t size_of_size = 8;
			constexpr uint32_t size_of_offset = 8;
			constexpr uint32_t size_of_id = 8;
			constexpr uint32_t size_of_module_id = 2;
			constexpr uint32_t size_of_section_data = 1+ size_of_offset + size_of_size;	//section_type (1 byte),offset(8 bytes), size (8 bytes)

			constexpr uint32_t size_of_constant_field_header = size_of_id + size_of_size;	//data can be infinity 
			constexpr uint32_t size_of_symbol_field = size_of_id + size_of_offset + size_of_module_id + sizeof(CompilationInfo::SymbolType);//19 bytes
			constexpr uint32_t command_size = 32;	//It's possible to use less, but the packaging will be more complicated.

			constexpr std::string_view module_prefix = "mslmodule";
			constexpr std::string_view module_extension = "mslco";
			constexpr std::string_view main_extension = "mslmo";

			constexpr std::string_view mo_magic = "MSLMO";
			constexpr std::string_view co_magic = "MSLCO";
			constexpr float FilesVersion = 1.0f;
		}

		enum SectionType : uint8_t 
		{
			LINKING_ORDER = 14,
			CONSTANTS,
			SYMBOLS,
			DEBUG,

		};



		class Packer	//Transforms ByteCommands to bytes array for saving in objects files
		{
			inline void WriteBytes(void* dest, void* src, size_t size, size_t& offset) {
				memcpy(dest, src, size);
				offset += size;
			}
			inline void WriteSectionInfo(char* dest, size_t& offset, SectionType type, size_t data_offset, size_t data_size) {

				WriteBytes(dest + offset, &type, sizeof(SectionType), offset);
				WriteBytes(dest + offset, &data_offset, MSLOData::size_of_offset, offset);
				WriteBytes(dest + offset, &data_size, MSLOData::size_of_size, offset);
			}
		public:
			std::pair<char*, size_t> ByterizeCompilationState(CompilationInfo::CompilationState* cs_state, Preprocessing::HandlingOrder& ho);
			std::pair<char*, size_t> ByterizeCommands(CompilationInfo::CompilationState* cs_state, BCommandsArray& commands, size_t module_id);


			bool SaveAsMSLMO(const fs::path& path, std::pair<char*, size_t> bytes, const std::string& file_name);	//path with
			bool SaveAsMSLCO(const fs::path& path, std::pair<char*, size_t> bytes, size_t module_id);

			Packer() = default;

			~Packer();
		};

	}
}