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

				void AnalyzeAPST(TokensGroup& node, PseudoTranslationState& pts);

				void HandleSimple(TokensGroup& node, PseudoTranslationState& pts);

				CompilationInfo::Values::ValueInfo GetValueInfoFromType(TokensGroup& node, PseudoTranslationState& pts);

			public:

				void AnalyzeExpression(std::vector<Tokenization::Token> tokens, PseudoTranslationState& pts);



			};
		}
	
	}
}