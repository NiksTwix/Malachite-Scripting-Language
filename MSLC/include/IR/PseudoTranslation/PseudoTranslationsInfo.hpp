#pragma once
#include "..\..\Definitions\ValueContainer.hpp"
#include <unordered_map> 

#include "..\..\Definitions\ChunkArray.hpp"
#include "..\..\CompilationInfo\CompilationState.hpp"


namespace MSLC 
{
	namespace IntermediateRepresentation
	{

		namespace Pseudo
		{
			enum class PseudoOpCode : uint16_t
			{
				Nop,
			};

			struct PseudoOperation
			{
				size_t arg_0;
				size_t arg_1;
				size_t arg_2;
				int debug_line;
				PseudoOpCode op_code;
				uint8_t flags;
			};

			using POperationArray = Definitions::ChunkArray<PseudoOperation>;

			

			struct PseudoTranslationState 
			{
				POperationArray pseudo_code{};
				CompilationInfo::CompilationState* cs_observer = nullptr;
			};

		}
	}
}