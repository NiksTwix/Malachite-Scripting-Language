#pragma once
#include "..\IR\ByteTranslation\ByteTranslationInfo.hpp"
#include "..\Preprocessing\AMGBuilder.hpp"


namespace MSLC 
{
	namespace Packing 
	{
		using namespace IntermediateRepresentation::Byte;
		
	    
	    
		namespace MSLOSizes		//in bytes
		{
			constexpr uint32_t mo_header_reserved_size = 500;
			constexpr uint32_t co_header_reserved_size = 100;

			constexpr uint32_t magic = 4;
			constexpr uint32_t version = 2;
			constexpr uint32_t flags = 2;
			constexpr uint32_t module_file_prefix = 32;//31 character + \0
			constexpr uint32_t module_extension = 10;
			constexpr uint32_t section_count = 4;

			constexpr uint32_t size_of_size = 8;
			constexpr uint32_t size_of_offset = 8;
			constexpr uint32_t size_of_id = 8;
			constexpr uint32_t size_of_module_id = 2;
			constexpr uint32_t size_of_section_data = 1+ size_of_offset + size_of_size;	//section_type (1 byte),offset(8 bytes), size (8 bytes)

			constexpr uint32_t size_of_constant_field_header = size_of_id + size_of_size;	//data can be infinity 
			constexpr uint32_t size_of_symbol_field = size_of_id + size_of_offset + size_of_module_id + sizeof(CompilationInfo::SymbolType);//19 bytes
			constexpr uint32_t command_size = 32;	//It's possible to use less, but the packaging will be more complicated.
		}

		enum SectionType : uint8_t 
		{
			LINKING_ORDER,
			CONSTANTS,
			SYMBOLS,
			DEBUG,

		};



		class Packer	//Transforms ByteCommands to bytes array for saving in objects files
		{
	

			float FilesVersion = 1.0f;

			char module_prefix[32] = "mslmodule";
			char module_extension[10] = "mslco";

			inline void WriteBytes(void* dest, void* src, size_t size, size_t& offset) {
				memcpy(dest, src, size);
				offset += size;
			}
			inline void WriteSectionInfo(void* dest, size_t& offset, SectionType type, size_t data_offset, size_t data_size) {
				WriteBytes(dest, &type, sizeof(SectionType), offset);
				WriteBytes(dest, &data_offset, MSLOSizes::size_of_offset, offset);
				WriteBytes(dest, &data_size, MSLOSizes::size_of_size, offset);
			}
		public:
			std::pair<char*, size_t> ByterizeCompilationState(CompilationInfo::CompilationState* cs_state, Preprocessing::HandlingOrder& ho);
			std::pair<char*, size_t> ByterizeCommands(CompilationInfo::CompilationState* cs_state, BCommandsArray& commands, size_t module_id);


			bool SaveAsMSLMO(const std::string path, std::pair<char*, size_t> bytes);
			bool SaveAsMSLCO(const std::string path, std::pair<char*, size_t> bytes);
		};

	}
}