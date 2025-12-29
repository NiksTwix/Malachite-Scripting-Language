#pragma once
#include "TokensInfo.hpp"
#include "..\Definitions\ValueContainer.hpp"
namespace MSLC 
{
	namespace Tokenization 
	{
		struct LexingState 
		{
			std::string undefined_token = std::string();
			int current_line = 0;
			int current_depth = 0;
			bool was_comment = false;
			bool was_neg_value = false;
			size_t current_index = 0;
			size_t module_id = 0;
		};


		class Lexer
		{
		private:
			///--------Methods

			Definitions::ValueType GetNumberValueType(LexingState& lex_state,const std::string& number);

			char GetNearCharWithoutSpaces(const std::string& text, size_t start_index, int step = 1);

			TokenType GetTokenType(LexingState& lex_state, const std::string& token);

			Definitions::ValueContainer GetTokenValue(LexingState& lex_state, const std::string& token);

			Token CreateToken(LexingState& lex_state,std::string operand);
			char ProcessCharLiteral(LexingState& lex_state,const std::string& str);
			std::string ProcessStringLiteral(LexingState& lex_state, const std::string& str);

			Token ProcessOperator(LexingState& lex_state,const std::string& text, std::vector<Token>& tokens);

			Token InsertOpEnd(LexingState& lex_state, std::vector<Token>& tokens, const std::string& text);

			bool IsUnary(const LexingState& state, const std::vector<Token>& tokens);
		public:
			std::vector<Token> ToTokens(std::string text, size_t module_id = 0);
		};
	}

}