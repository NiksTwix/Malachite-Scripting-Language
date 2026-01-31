#pragma once
#include "PseudoTranslationsInfo.hpp"


namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			constexpr size_t tabs_count = 2;

			class CompilationStateStringSerializator 
			{
			private:
				std::string SerializeValueInfoFlags(uint8_t flags);
			public:
				std::string Serialize(CompilationInfo::CompilationState& state);
			};



			class PseudoOperationsStringSerializator 
			{
			private:
				std::unordered_map<PseudoOpCode, std::string> codes;
				int column_size = 20;
			public:
				PseudoOperationsStringSerializator();
				std::string Serialize(const PseudoOperation& operation);
				std::string Serialize(POperationArray& array);
			};


		}
	}
}