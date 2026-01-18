#pragma once
#include "..\..\..\include\Tokenization\TokensInfo.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			enum class GroupType 
			{
				Simple,		//alone token

				//Complex
				FunctionCall,
				AttributeUsing,
				Argument,

				TypeConvertion,		//x to Vector	
			};

			struct TokensGroup 
			{

				Tokenization::Token simple;
				std::vector<Tokenization::Token> complex;

				GroupType type = GroupType::Simple;


				bool IsSimple() const { return type == GroupType::Simple; }

				TokensGroup() 
				{
					type = GroupType::Simple;
					simple = Tokenization::Token();
				}
				TokensGroup(GroupType type) 
				{
					if (type == GroupType::Simple) simple = Tokenization::Token();
					else complex = std::vector<Tokenization::Token>();
				}
			};


			class PostfixBuilder 
			{
			public:
				TokensGroup BuildPostfix(std::vector<Tokenization::Token> tokens);
			};
		}
	}
}
