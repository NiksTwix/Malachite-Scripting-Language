#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <functional>


namespace FileStructs
{
	constexpr std::string_view msli_magic = "MSLI";

}




namespace fs = std::filesystem;

enum class VMs : uint8_t
{
	INVALID = 0,
	MSLVM_1 = 1,
};


struct static_bpointer
{
	char* ptr = nullptr;
	size_t bytes_size = 0;

	static_bpointer() = default;

	explicit static_bpointer(size_t bytes) {
		allocate(bytes);
	}

	bool allocate(size_t bytes) {
		ptr = static_cast<char*>(calloc(bytes, 1));
		if (!ptr) return false;
		bytes_size = bytes;
		return true;
	}

	void release() {
		if (ptr) ::free(ptr);
		ptr = nullptr;
		bytes_size = 0;
	}

	bool is_valid() { return ptr != nullptr && bytes_size != 0; }
};

struct InterpretationState 
{
	VMs vm_type = VMs::INVALID;

	size_t file_size = 0;

	float file_version = 1.0f;

	uint16_t flags = 0;

	static_bpointer rod_section;
	size_t rod_offset = 0;
	static_bpointer code_section;
	size_t code_offset = 0;
	bool broken_file = false;

};


