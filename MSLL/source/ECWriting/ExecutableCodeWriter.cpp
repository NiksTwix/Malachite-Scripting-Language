#include "..\..\include\ECWriting\ExecutableCodeWriter.hpp"

#include <fstream>
#include <iostream>
namespace MSLL
{
	bool ECWriter::SaveAsMSLI(fs::path directory, fs::path name, LinkDefinitions::ExecutionData& data, bool free_ed)
	{

		size_t buffer_size = MSLI::header_size + data.aligned_rod_size + data.code.bytes_size;

		
		char* buffer = static_cast<char*>(calloc(buffer_size, 1));

		//header

		size_t offset = 0;

		memcpy(buffer + offset, MSLI::magic.data(), MSLI::magic.size()); offset += MSLI::magic.size();
		memcpy(buffer + offset, &MSLI::version, sizeof(MSLI::version)); offset += sizeof(MSLI::version);

		//skip flags

		offset += sizeof(uint16_t);

		memcpy(buffer + offset, &data.vm_type, sizeof(data.vm_type)); offset += sizeof(data.vm_type);	//vm_version


		size_t data_offset = MSLI::header_size;

		//rod offset
		memcpy(buffer + offset, &MSLI::header_size, sizeof(MSLI::header_size)); offset += sizeof(MSLI::header_size);		//Now rod is behind the header section
		//rod size
		memcpy(buffer + offset, &data.aligned_rod_size, sizeof(data.aligned_rod_size)); offset += sizeof(data.aligned_rod_size);

		data_offset += data.aligned_rod_size;

		//code offset
		memcpy(buffer + offset, &data_offset, sizeof(data_offset)); offset += sizeof(data_offset);		//Now rod is behind the header section
		//code size
		memcpy(buffer + offset, &data.code.bytes_size, sizeof(data.code.bytes_size)); offset += sizeof(data.code.bytes_size);

		offset = MSLI::header_size;

		memcpy(buffer + offset, data.read_only_data.ptr, data.read_only_data.bytes_size); offset += data.aligned_rod_size;
		memcpy(buffer + offset, data.code.ptr, data.code.bytes_size); offset += data.code.bytes_size;

		//for (size_t i = 100; i < 116; i++) 
		//{
		//	std::cout << (size_t)*(buffer + i) << " ";
		//
		//}
		if (free_ed) data.Free();

		//File saving

		
		std::ofstream file_str(directory / name, std::ios::binary);

		if (file_str.is_open()) {
			
			file_str.write(buffer, buffer_size);

			free(buffer);
			file_str.close();
		}
		else {
			free(buffer);
			return false;
		}

		return true;
	}
}