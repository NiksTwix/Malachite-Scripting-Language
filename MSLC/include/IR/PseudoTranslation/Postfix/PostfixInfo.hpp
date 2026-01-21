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

				Operation,	//Simple and complex!
				Declaration,	//Of variable
				Type,		//Vector or Vector* or Vector&
				TypeCast,		//to
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
				TokensGroup(std::vector<TokensGroup>& tokens, GroupType group_type)
				{
					type = group_type;
					complex = tokens;
				}
				TokensGroup(TokensGroup&& simple_group,std::vector<TokensGroup>&& tokens, GroupType group_type)
				{
					type = group_type;
					complex = std::move(tokens);
					simple = std::move(simple_group.simple);
				}
				TokensGroup(TokensGroup& simple_group, std::vector<TokensGroup>& tokens, GroupType group_type)
				{
					type = group_type;
					complex = tokens;
					simple = simple_group.simple;
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