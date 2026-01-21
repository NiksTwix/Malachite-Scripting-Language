#pragma once
#include "PostfixInfo.hpp"
#include "..\PseudoTranslationsInfo.hpp"
namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			class APSTBuilder 
			{
			private:
				TokensGroup HandleOperator(std::vector<TokensGroup>& stack,TokensGroup& _operator);
				TokensGroup HandleToOperator(std::vector<TokensGroup>& stack, TokensGroup& to_op);
			public:
				TokensGroup BuildAPST(TokensGroup& root);
			};
		}
	}
}