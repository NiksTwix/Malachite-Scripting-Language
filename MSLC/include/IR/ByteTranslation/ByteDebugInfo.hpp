#pragma once
#include "ByteTranslationInfo.hpp"



namespace MSLC {
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			constexpr size_t tabs_count = 2;
			class CommandStringSerializer
			{
			private:
				std::unordered_map<ByteOpCode, std::string> bopts_table_;
				std::unordered_map<CommandSource, std::string> sts_table_;
				std::unordered_map<ByteCommand::Flag, std::string> cfts_table_;
			public:
				CommandStringSerializer();

				std::string Serialize(ByteCommand& command);

			};
		}
	}
}