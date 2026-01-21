#pragma once
#include "PostfixInfo.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			class APSTBuilder 
			{
			public:
				void BuildAPST(TokensGroup& tokens_group);
			};
		}
	}
}