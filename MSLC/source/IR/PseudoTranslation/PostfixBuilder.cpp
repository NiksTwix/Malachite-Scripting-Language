#include "..\..\..\include\IR\PseudoTranslation\PostfixBuilder.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			TokensGroup PostfixBuilder::BuildPostfix(std::vector<Tokenization::Token> tokens)
			{
				TokensGroup postfix(GroupType::Simple);
				return postfix;
			}
		}
	}
}