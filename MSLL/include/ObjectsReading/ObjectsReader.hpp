#pragma once
#include "LinkDefinitions.hpp"
#include <memory>
#include <string>
#include <iostream>
namespace MSLL
{
	class ObjectsReader {
	private:

		void HandleLOSection(LinkDefinitions::static_bpointer bytes_buffer, std::shared_ptr<LinkDefinitions::LinkingState> state, size_t section_offset, size_t section_size);
		void HandleConstantsSection(LinkDefinitions::static_bpointer  bytes_buffer, std::shared_ptr<LinkDefinitions::LinkingState> state, size_t section_offset, size_t section_size);
		void HandleSymbolSection(LinkDefinitions::static_bpointer  bytes_buffer, std::shared_ptr<LinkDefinitions::LinkingState> state, size_t section_offset, size_t section_size);
		void HandleDebugSection(LinkDefinitions::static_bpointer  bytes_buffer, std::shared_ptr<LinkDefinitions::LinkingState> state, size_t section_offset, size_t section_size);
	public:
		std::shared_ptr<LinkDefinitions::LinkingState> DeserializeMO(LinkDefinitions::static_bpointer  bytes_buffer);
		std::shared_ptr<LinkDefinitions::CommandsPool> DeserializeCO(LinkDefinitions::static_bpointer  bytes_buffer);

		LinkDefinitions::static_bpointer  ReadFile(const std::string& path);
	};
}