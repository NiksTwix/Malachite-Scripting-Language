#include "..\..\include\mslvm\mslvm.hpp"
#include <fstream>


namespace MSLVM 
{
	bool MSLVM::VirtualMachine::LoadScript(const std::string& path)
	{
		//MSLI reading
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			state.error_stack.push(ErrorFrame(0, ErrorCode::InvalidFilePath));
			return false;
		}
		file.seekg(0, std::ios::end);
		size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		char* file_stream = static_cast<char*>(calloc(fileSize, 1));

		file.read(file_stream, fileSize);

		file.close();

		//Deserialization

		bool magic_check = memcmp(file_stream, msli_magic.data(), msli_magic.size());

		if (magic_check) 
		{
			state.error_stack.push(ErrorFrame(0, ErrorCode::InvalidMagic));
			return false;
		}

		size_t offset = msli_magic.size();

		bool version_check = memcmp(file_stream+offset, &msli_version, sizeof(msli_version)); offset += sizeof(msli_version);

		if (version_check) 
		{
			//...
		}

		uint16_t flags;

		memcpy(file_stream + offset, &flags, sizeof(flags)); offset += sizeof(flags);
		
		//HandleFlags()

		bool vm_version_check = memcmp(file_stream + offset, &vm_version_code, sizeof(vm_version_code)); offset += sizeof(vm_version_code);

		if (vm_version_check)
		{
			state.error_stack.push(ErrorFrame(0, ErrorCode::AnotherVMVersion));
			return false;
		}

		size_t rod_offset;
		size_t rod_size;

		memcpy(&rod_offset, file_stream + offset, sizeof(rod_offset)); offset += sizeof(rod_offset);
		memcpy(&rod_size, file_stream + offset, sizeof(rod_size)); offset += sizeof(rod_size);

		size_t code_offset;
		size_t code_size;

		memcpy(&code_offset,file_stream + offset, sizeof(code_offset)); offset += sizeof(code_offset);
		memcpy(&code_size  ,file_stream + offset, sizeof(code_size)); offset += sizeof(code_size);

		bool checking = fileSize >= (code_offset + code_size);

		if (!checking) 
		{
			state.error_stack.push(ErrorFrame(0, ErrorCode::InvalidFile));
			return false;
		}

		state.memory.Allocate(code_size, rod_size);	//allocate memory
		state.memory.LoadCode(file_stream + code_offset, code_size);
		
		state.memory.LoadROD(file_stream + rod_offset, rod_size);


		free(file_stream);

		return true;
	}
}