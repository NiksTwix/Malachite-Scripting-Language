#include "..\..\include\Tokenization\Lexer.hpp"
#include <unordered_set>


namespace MSLC
{
	namespace Tokenization
	{
		Definitions::ValueType Lexer::GetNumberValueType(LexingState& lex_state, const std::string& number)
		{
			//StringsOperations::IsNumber has been just called early
			bool has_point = number.find('.') != std::string::npos;

			if (has_point) return Definitions::ValueType::REAL;

			try {
				if (lex_state.was_neg_value) {
					// Negative value - always INT
					return Definitions::ValueType::INT;
				}
				else {
					// Positive value - check interval
					uint64_t value = std::stoull(number);

					// If number is not so big for int64_t - make INT, in another case -  UINT
					if (value <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
						return Definitions::ValueType::INT;
					}
					else {
						return Definitions::ValueType::UINT;
					}
				}
			}
			catch (const std::out_of_range&) {
				// If number is bigger than uint64_t's max value
				Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Number value is greater(less) than max(min) permitted.", Diagnostics::MessageType::TypeError, Diagnostics::SourceCode, lex_state.current_line));
				return Definitions::ValueType::VOID;
			}
			return Definitions::ValueType::UINT;
		}
		char Lexer::GetNearCharWithoutSpaces(const std::string& text, size_t start_index, int step)

		{
			std::unordered_set<char> chars = { ' ','\t','\r','\n' };
			int i = start_index + step;
			while (i >= 0 && i < text.size())
			{
				if (!chars.count(text[i])) return text[i];
				i += step;
			}

			return '\0';
		}
		TokenType Lexer::GetTokenType(LexingState& lex_state, const std::string& token) //Простой и быстрый способ для получения токенов
		{
			auto type = TokensTypeTable::Get().GetTokenType(token);
			if (type != TokenType::UNDEFINED)
			{
				return type;
			}

			if (token[0] == '@' || token[0] == '#') return TokensTypeTable::Get().GetTokenType(token.substr(1,token.size()-1));	// Attribute or directive

			if (Strings::StringOperations::IsNumber(token)) return TokenType::LITERAL;

			if (token.size() >= 2 && token[0] == '"' && token[token.size() - 1] == '"') return TokenType::LITERAL;
			if (token.size() >= 2 && token[0] == '\'' && token[token.size() - 1] == '\'') return TokenType::LITERAL;

			return TokenType::IDENTIFIER;
		}
		Definitions::ValueContainer Lexer::GetTokenValue(LexingState& lex_state, const std::string& token)		//Транслирует в строки/цифры и тд
		{
			if (token == Literals::w_true) return true;
			else if (token == Literals::w_false) return false;

			if (Strings::StringOperations::IsNumber(token))	//TODO type management
			{
				auto value_type = GetNumberValueType(lex_state,token);
				bool inv_copy = lex_state.was_neg_value;
				lex_state.was_neg_value = false;
				switch (value_type)
				{
				case  Definitions::ValueType::VOID:
					return 1.0;		//safe for all operations
				case  Definitions::ValueType::INT:
					if (inv_copy) return -std::stoll(token);
					return std::stoll(token);
				case  Definitions::ValueType::UINT:
					if (inv_copy) {
						// Negative UINT? Its a error
						Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Unsigned integer cannot be negative.", Diagnostics::MessageType::TypeError, Diagnostics::SourceCode, lex_state.current_line));
						return 1.0;
					}
					return std::stoull(token);
				case  Definitions::ValueType::REAL:
					if (inv_copy) return -std::stod(token);
					return std::stod(token);
				default:
					break;
				}
				return std::stod(token);
			}
			if (token.size() >= 2 && token[0] == '\'' && token.back() == '\'') {
				return ProcessCharLiteral(lex_state,token.substr(1, token.size() - 2));
			}
			if (token.size() >= 2 && token[0] == '"' && token.back() == '"') {
				return ProcessStringLiteral(lex_state,token.substr(1, token.size() - 2));
			}
			return token;
		}
		Token Lexer::CreateToken(LexingState& lex_state, std::string operand)
		{
			Token t;
			t.line = lex_state.current_line;
			t.module_id = lex_state.module_id;
			t.type = GetTokenType(lex_state,operand);
			t.value = GetTokenValue(lex_state,operand);
			
			return t;
		}
		char Lexer::ProcessCharLiteral(LexingState& lex_state, const std::string& str)
		{
			int size = str.size();
			if (size > 4 || size == 0)//Not escape and not char		//TODO Create Compiler Errors
			{
				Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Invalid char.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, lex_state.current_line));
				return '\0';
			}
			if (size == 1) return str[0];
			if (size == 2 && str[0] == '\\')	//Escape
			{
				switch (str[1]) {
				case 'n':return '\n';
				case 't':return '\t';
				case '"':return '"';
				case '\'': return '\'';
				case '\\': return'\\';
				}
			}
			Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Invalid char.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, lex_state.current_line));
			return '\0';
		}
		std::string Lexer::ProcessStringLiteral(LexingState& lex_state, const std::string& str)
		{
			std::string result;
			result.reserve(str.size()); //Memory optimization

			for (size_t i = 0; i < str.size(); ++i) {
				if (str[i] == '\\' && i + 1 < str.size()) {
					switch (str[i + 1]) {
					case 'n': result += '\n'; break;
					case 't': result += '\t'; break;
					case '"': result += '"'; break;  // Its needed
					case '\'': result += '\''; break;  // Its needed
					case '\\': result += '\\'; break;
					case 'r': result += '\r'; break;
					case 'b': result += '\b'; break;
					case 'f': result += '\f'; break;
					case 'v': result += '\v'; break;
					case '0': result += '\0'; break;
					default:
						// Undefined escape-sequence - dont touch
						result += str[i];
						continue; // dont skip next symbol
					}
					i++; // Skip handled symbol
				}
				else {
					result += str[i];
				}
			}
			return result;
		}
		bool Lexer::IsUnary(const LexingState& state, const std::vector<Token>& tokens)
		{
			if (tokens.size() == 0)  return true;

			bool wasnt_closings_brackets = tokens.back().value.strVal != ")" && tokens.back().value.strVal != "]";	// && tokens.back().value.strVal != "}"
			bool wasnt_expression = tokens.back().type != TokenType::IDENTIFIER && tokens.back().type != TokenType::LITERAL;
			return wasnt_closings_brackets && wasnt_expression;
		}

		Token Lexer::ProcessOperator(LexingState& lex_state, const std::string& text, std::vector<Token>& tokens)
		{
			//Multichar operators: <<= and etc

			std::string operator_;
			bool is_unary = IsUnary(lex_state,tokens);
			for (; lex_state.current_index < text.size(); lex_state.current_index++)
			{
				char c = text[lex_state.current_index];
				if (GetTokenType(lex_state,std::string(1, c)) == TokenType::OPERATOR)	operator_.push_back(c);
				else break;
			}
			if (is_unary) operator_.push_back('u');
			return Token(operator_,TokensTypeTable::Get().GetTokenType(operator_), lex_state.current_line, lex_state.module_id);
		}

		Token Lexer::InsertOpEnd(LexingState& lex_state, std::vector<Token>& tokens, const std::string& text)
		{
			if (tokens.size() == 0)return Token(TokenType::UNDEFINED, lex_state.current_line, lex_state.module_id);
			if (text[lex_state.current_index] == ';') return Token((uint64_t)CompilationLabel::OPERATION_END, TokenType::COMPILATION_LABEL,lex_state.current_line,lex_state.module_id);

			if (tokens.back().type == TokenType::COMPILATION_LABEL || tokens.back().value.strVal == "{" && lex_state.undefined_token.empty()) return Token(TokenType::UNDEFINED, lex_state.current_line, lex_state.module_id);

			if (tokens.back().type != TokenType::COMPILATION_LABEL && tokens.back().value.strVal != "}" && text[lex_state.current_index] == '}')
			{
				return Token((uint64_t)CompilationLabel::OPERATION_END, TokenType::COMPILATION_LABEL, lex_state.current_line,lex_state.module_id);
			}

			if (text[lex_state.current_index] == '\n' && !(GetNearCharWithoutSpaces(text, lex_state.current_index, 1) == '{' || GetNearCharWithoutSpaces(text, lex_state.current_index, -1) == '}'))
			{
				return Token((uint64_t)CompilationLabel::OPERATION_END, TokenType::COMPILATION_LABEL, lex_state.current_line, lex_state.module_id);
			}

			return Token(TokenType::UNDEFINED, lex_state.current_line, lex_state.module_id);
		}
		
		std::vector<Token> Lexer::ToTokens(std::string text, size_t module_id)
		{
			std::vector<Token> result;
			LexingState state;
			state.current_line = 1;
			bool in_string = false;
			result.reserve(text.size() / 4);
			bool in_symbol_literal = false;

			for (state.current_index = 0; state.current_index < text.size(); state.current_index++)
			{
				char c = text[state.current_index];
				if (c == '\r') continue;
				if (c == '\n')
				{
					state.current_line++;
				}
				// Simple line comments
				if (!in_string && !in_symbol_literal)
				{
					if (c == '/' && state.current_index + 1 < text.size() && text[state.current_index + 1] == '/') {
						while (state.current_index < text.size() && text[state.current_index] != '\n')
						{
							state.current_index++;
						}
						state.was_comment = true;
						state.current_index--;	//чтобы вернуть \n
						continue;
					}
					//Multilines comments
					if (c == '/' && state.current_index + 1 < text.size() && text[state.current_index + 1] == '*') {
						char prev = text[state.current_index];
						state.current_index++;
						while (state.current_index < text.size())
						{
							if (prev == '*' && text[state.current_index] == '/')
							{
								state.was_comment = true;
								break;
							}
							prev = text[state.current_index];
							state.current_index++;
						}
						if (!state.was_comment) Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Excepts \"*/\".", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.current_line));
						continue;
					}

					if (c == ' ' || c == '\n' || c == '\t')
					{
						if (!state.undefined_token.empty())
						{
							result.push_back(CreateToken(state,state.undefined_token));
						}
						state.undefined_token.clear();

					}

					auto t = InsertOpEnd(state,result, text);
					if (t.type != TokenType::UNDEFINED)
					{
						if (!state.undefined_token.empty())
						{
							result.push_back(CreateToken(state,state.undefined_token));
							state.undefined_token.clear();
						}
						result.push_back(t);
						if (c == ';')continue;
					}
					if (state.was_comment) state.was_comment = false;


					if (GetTokenType(state,std::string(1, c)) == TokenType::OPERATOR)
					{
						std::string operator_ = std::string({ c });
						if (!state.undefined_token.empty())	//Сразу добавляем токен перед ним, если есть
						{
							result.push_back(CreateToken(state,state.undefined_token));
							state.undefined_token.clear();
						}

						if (state.current_index < text.size() - 1 && GetTokenType(state,std::string({ c, text[state.current_index + 1] })) == TokenType::LITERAL && (result.size() == 0 || (result.back().type != TokenType::LITERAL && result.back().type != TokenType::IDENTIFIER)))  //if its number
						{
							state.was_neg_value = true;
							continue;
						}
						Token t = ProcessOperator(state,text, result);
						state.current_index--;
						if (t.type == TokenType::UNDEFINED)
						{
							Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Invalid operator \"" + t.value.strVal + "\".", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.current_line));
							continue;
						}
						result.push_back(t);
						continue;
					}


					if (GetTokenType(state,std::string(1, c)) == TokenType::DELIMITER)
					{
						std::string delimiter = std::string({ c });
						if (c == '.' && Strings::StringOperations::IsNumber(state.undefined_token))
						{
							state.undefined_token.push_back(c);
							continue;
						}
						if (!state.undefined_token.empty())	//Сразу добавляем токен перед ним, если есть
						{
							result.push_back(CreateToken(state,state.undefined_token));
							state.undefined_token.clear();
						}
						Token t;
						t.line = state.current_line;
						t.type = TokenType::DELIMITER;
						t.value = delimiter;
						t.module_id = state.module_id;
						if (c == '{') state.current_depth++;
						if (c == '}')
						{
							state.current_depth--;
						}

						result.push_back(t);
						continue;
					}
				}

				if (c == '\'' && state.undefined_token.empty() && !in_string)
				{
					in_symbol_literal = true;
					state.undefined_token.push_back(c);
					continue;
				}
				if (c == '\'' && !state.undefined_token.empty() && in_symbol_literal)
				{
					in_symbol_literal = false;
					state.undefined_token.push_back(c);
					result.push_back(CreateToken(state,state.undefined_token));
					state.undefined_token.clear();
					continue;
				}
				if (c == '\"' && state.undefined_token.empty() && !in_symbol_literal)
				{
					in_string = true;
					state.undefined_token.push_back(c);
					continue;
				}
				if (c == '\"' && !state.undefined_token.empty() && in_string)
				{
					in_string = false;
					state.undefined_token.push_back(c);
					result.push_back(CreateToken(state,state.undefined_token));
					state.undefined_token.clear();
					continue;
				}
				if ((c == ' ' || c == '\n' || c == '\t') && !in_string && !in_symbol_literal) continue;
				state.undefined_token.push_back(c);
			}

			if (!state.undefined_token.empty())
			{
				Token t;
				t.line = state.current_line;
				t.type = GetTokenType(state,state.undefined_token);
				t.value = GetTokenValue(state,state.undefined_token);
				t.module_id = state.module_id;
				result.push_back(t);
				result.push_back(Token((uint64_t)CompilationLabel::OPERATION_END, TokenType::COMPILATION_LABEL, state.current_line, state.module_id));
			}

			return result;
		}
	}
}