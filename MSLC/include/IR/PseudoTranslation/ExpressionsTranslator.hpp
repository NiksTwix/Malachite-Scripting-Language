#pragma once
#include "PseudoTranslationsInfo.hpp"
#include "Postfix\PostfixBuilder.hpp"
namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo 
		{
			class ExpressionsTranslator 
			{
			private:
				PostfixBuilder postfix_builder;
			public:

				Definitions::ChunkArray<PseudoOperation> AnalyzeExpression(std::vector<Tokenization::Token> tokens, PseudoTranslationState& pts);

			};
		}
	
	}
}