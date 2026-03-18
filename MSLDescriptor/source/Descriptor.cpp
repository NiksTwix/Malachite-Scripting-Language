#include "..\include\Descriptor.hpp"
#include <string>

InterpretationState Descriptor::LoadI(std::filesystem::path filep)
{
	InterpretationState state;

	std::ifstream file(filep, std::ios::binary);

	if (!file)
	{
		std::cerr << "Read file error. Path:" << filep << "\n";
		state.broken_file = true;
		return state;
	}

	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();		//file.tellg returns position of file pointer
	file.seekg(0, std::ios::beg);

	static_bpointer X(size);

	file.read(X.ptr, size);

	file.close();

	
	state.file_size = size;

	// parsing

	char* file_stream = X.ptr;

	bool magic_check = memcmp(file_stream, FileStructs::msli_magic.data(), FileStructs::msli_magic.size());

	if (magic_check)
	{
		state.broken_file = true;
		std::cerr << "Its not file for interpretation.\n";
		return state;
	}

	size_t offset = FileStructs::msli_magic.size();



	memcpy(&state.file_version, file_stream + offset, sizeof(state.file_version)); offset += sizeof(state.file_version);

	memcpy(&state.flags, file_stream + offset, sizeof(state.flags)); offset += sizeof(state.flags);

	memcpy(&state.vm_type, file_stream + offset, sizeof(state.vm_type)); offset += sizeof(state.vm_type);

	size_t rod_offset;
	size_t rod_size;

	memcpy(&rod_offset, file_stream + offset, sizeof(rod_offset)); offset += sizeof(rod_offset);
	memcpy(&rod_size, file_stream + offset, sizeof(rod_size)); offset += sizeof(rod_size);

	size_t code_offset;
	size_t code_size;

	memcpy(&code_offset, file_stream + offset, sizeof(code_offset)); offset += sizeof(code_offset);
	memcpy(&code_size, file_stream + offset, sizeof(code_size)); offset += sizeof(code_size);

	bool checking = size >= (code_offset + code_size);

	if (!checking)
	{
		state.broken_file = true;
		std::cerr << "Invalid offsets and sizes in file.\n";
		return state;
	}

	state.code_section.allocate(code_size);
	
	memcpy(state.code_section.ptr, file_stream + code_offset, code_size);
	state.rod_section.allocate(rod_size);
	
	memcpy(state.rod_section.ptr, file_stream + rod_offset, rod_size);

	X.release();

	state.rod_offset = rod_offset;
	state.code_offset = code_offset;

	return state;
}


