#pragma once
#include "PseudoTranslationsInfo.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo 
		{
			class ExpressionsTranslator 
			{
			public:

				Definitions::ChunkArray<PseudoOperation> AnalyzeExpression(std::vector<Tokenization::Token> tokens, PseudoTranslationState& pts);

			};
		}
	
	}
}