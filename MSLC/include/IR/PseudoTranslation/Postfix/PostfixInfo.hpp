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
				FunctionCall,	//first in complex is identifier another are arguments
				DataAccess,	//first in complex is identifier another are arguments
				DataAccessChain,	//Chain of DataAccesses: DataAccess(identificator), DataAccess()

				ArrayLiteral,	//[10,10,20]


				AttributeUsing,	//simple - identificator, complex -arguments
				Argument,

				Operation,	//Simple and complex!
				Declaration,	//Of variable
				Type,		//Vector or Vector* or Vector&
				TypeCast,		//to
				NewExpression,	//new

				QualifiedName,	//Namespace::Type::StaticField

			};

			struct TokensGroup
			{

				Tokenization::Token simple;
				std::vector<TokensGroup> complex;

				GroupType type = GroupType::Simple;

				Diagnostics::DeclaringPlace declaring_place;

				bool IsSimple() const { return type == GroupType::Simple; }
				bool IsSimpleOrQN() const {
					return type == GroupType::QualifiedName ||
						type == GroupType::ArrayLiteral ||
						type == GroupType::Simple;
				}
				TokensGroup()
				{
					type = GroupType::Simple;
				}
				TokensGroup(std::vector<TokensGroup>& tokens)
				{
					type = GroupType::Root;
					complex = tokens;
					if (!tokens.empty()) declaring_place = tokens.front().declaring_place;
				}
				TokensGroup(std::vector<TokensGroup>& tokens, GroupType group_type)
				{
					type = group_type;
					complex = tokens;
					if (!tokens.empty()) declaring_place = tokens.front().declaring_place;
				}
				TokensGroup(TokensGroup simple_group, std::vector<TokensGroup> tokens, GroupType group_type)
				{
					type = group_type;
					declaring_place = simple_group.declaring_place;
					complex = tokens;
					simple = simple_group.simple;
				}
				TokensGroup(Tokenization::Token simple, std::vector<TokensGroup> tokens, GroupType group_type)
				{
					type = group_type;
					complex = tokens;
					declaring_place = simple.declaring_place;
					this->simple = simple;
				}
				TokensGroup(Tokenization::Token token)
				{
					type = GroupType::Simple;
					declaring_place = token.declaring_place;
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