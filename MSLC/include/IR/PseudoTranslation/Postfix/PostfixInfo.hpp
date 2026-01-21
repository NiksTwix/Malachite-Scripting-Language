#pragma once
#include "..\..\..\..\include\Tokenization\TokensInfo.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			enum class GroupType
			{
				Empty,
				Root,
				Simple,		//alone token

				//Complex
				FunctionCall,	//simple - identificator, complex -arguments
				ArrayAccess,	//simple - identificator, complex -arguments
				AttributeUsing,	//simple - identificator, complex -arguments
				Argument,

				Type,		//Vector or Vector* or Vector&
			};

			struct TokensGroup
			{

				Tokenization::Token simple;
				std::vector<TokensGroup> complex;

				GroupType type = GroupType::Simple;



				bool IsSimple() const { return type == GroupType::Simple; }

				TokensGroup()
				{
					type = GroupType::Simple;
				}
				TokensGroup(std::vector<TokensGroup>& tokens)
				{
					type = GroupType::Root;
					complex = tokens;
				}
				TokensGroup(Tokenization::Token token)
				{
					type = GroupType::Simple;
					simple = token;
				}
				TokensGroup(GroupType type) : type(type)
				{
					if (type == GroupType::Simple) simple = Tokenization::Token();
					else complex = std::vector<TokensGroup>();
				}
			};




		}
	}
}