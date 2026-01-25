#pragma once
#include "PseudoTranslationsInfo.hpp"
#include "Postfix\PostfixBuilder.hpp"
#include "Postfix\APSTBuilder.hpp"
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
				APSTBuilder apst_builder;


				Definitions::ChunkArray<PseudoOperation> AnalyzeAPST(TokensGroup& node, PseudoTranslationState& pts);

			public:

				Definitions::ChunkArray<PseudoOperation> AnalyzeExpression(std::vector<Tokenization::Token> tokens, PseudoTranslationState& pts);



			};
		}
	
	}
}