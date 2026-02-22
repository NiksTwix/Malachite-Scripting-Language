#pragma once
#include "ObjectsInfo.hpp"
#include <memory>
#include <string>
namespace MSLL
{
	class ObjectsReader {
	private:

		void HandleLOSection(std::pair<char*, size_t> bytes_buffer, std::shared_ptr<ObjectsInfo::LinkingState> state, size_t section_offset, size_t section_size);
		void HandleConstantsSection(std::pair<char*, size_t> bytes_buffer, std::shared_ptr<ObjectsInfo::LinkingState> state, size_t section_offset, size_t section_size);
		void HandleSymbolSection(std::pair<char*, size_t> bytes_buffer, std::shared_ptr<ObjectsInfo::LinkingState> state, size_t section_offset, size_t section_size);
		void HandleDebugSection(std::pair<char*, size_t> bytes_buffer, std::shared_ptr<ObjectsInfo::LinkingState> state, size_t section_offset, size_t section_size);
	public:
		std::shared_ptr<ObjectsInfo::LinkingState> DeserializeMO(std::pair<char*, size_t> bytes_buffer);


		std::pair<char*, size_t> ReadFile(const std::string& path);
	};
}