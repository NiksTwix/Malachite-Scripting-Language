#pragma once
#include "PostfixInfo.hpp"
#include "..\PseudoTranslationsInfo.hpp"
namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{



			class PostfixBuilder 
			{
				GroupType IdentificateGroupType(const std::vector<TokensGroup>& tokens, size_t current_index);
				TokensGroup HandleFuncCall(const std::vector<TokensGroup>& tokens, size_t& current_index);
				TokensGroup HandleDataAccessOrArrayLiteral(const std::vector<TokensGroup>& tokens, size_t& current_index, GroupType gtype);


				std::vector<TokensGroup> Preprocess(const std::vector<Tokenization::Token>& tokens);
				TokensGroup BuildPostfix(const std::vector<TokensGroup>& tokens);
			public:

				

				PostfixBuilder();
				TokensGroup Build(const std::vector<Tokenization::Token>& tokens);
			};
		}
	}
}
