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
				GroupType IdentificateGroupType(const std::vector<Tokenization::Token>& tokens, size_t current_index);
				TokensGroup HandleFuncCall(const std::vector<Tokenization::Token>& tokens, size_t& current_index);

			public:

				

				PostfixBuilder();
				TokensGroup BuildPostfix(const std::vector<Tokenization::Token>& tokens);
			};
		}
	}
}
