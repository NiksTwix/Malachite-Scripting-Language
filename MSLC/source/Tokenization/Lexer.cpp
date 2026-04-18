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
				Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Number value is greater(less) than max(min) permitted.", Diagnostics::MessageType::TypeError, Diagnostics::SourceCode, lex_state.declaring_place));
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
			auto type = TokensInfoTable::Get().GetTokenType(token);
			if (type != TokenType::UNDEFINED)
			{
				return type;
			}

			if (token[0] == '#')
			{
				type = TokensInfoTable::Get().GetTokenType(token.substr(1, token.size() - 1));	//If exists - its directive, else - directive's call
				return type == TokenType::UNDEFINED ? TokenType::PREPROCESSOR_DIRECTIVE_CALL: type;
			}
			if (token[0] == '@') 
			{
				return TokensInfoTable::Get().GetTokenType(token.substr(1, token.size() - 1));	// Attribute 
			}
			if (Strings::StringOperations::IsNumber(token)) return TokenType::LITERAL;

			if (token.size() >= 2 && token[0] == '"' && token[token.size() - 1] == '"') return TokenType::LITERAL;
			if (token.size() >= 2 && token[0] == '\'' && token[token.size() - 1] == '\'') return TokenType::LITERAL;

			return TokenType::IDENTIFIER;
		}
		Definitions::ValueContainer Lexer::GetTokenValue(LexingState& lex_state, const std::string& token, TokenType& type)	//Транслирует в строки/цифры и тд
		{
			if (type == TokenType::CONST_LITERAL) 
			{
				type = TokenType::LITERAL;	//false -> 0| CONST_LITERAL -> LITERAL
				return TokensInfoTable::Get().GetConstLiteralValue(token);
			}
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
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Unsigned integer cannot be negative.", Diagnostics::MessageType::TypeError, Diagnostics::SourceCode, lex_state.declaring_place));
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
			if (token[0] == '@' || token[0] == '#') 
			{
				return token.substr(1, token.size() - 1);
			}
			return token;
		}
		Token Lexer::CreateToken(LexingState& lex_state, std::string operand)
		{
			Token t;
			t.declaring_place = lex_state.declaring_place;
			t.type = GetTokenType(lex_state,operand);
			t.value = GetTokenValue(lex_state,operand,t.type);
			
			return t;
		}
		char Lexer::ProcessCharLiteral(LexingState& lex_state, const std::string& str)
		{
			int size = str.size();
			if (size > 4 || size == 0)//Not escape and not char		//TODO Create Compiler Errors
			{
				Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid char.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, lex_state.declaring_place));
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
			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid char.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, lex_state.declaring_place));
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
		
		UnaryType Lexer::IsUnary(const LexingState& state,const std::string& text,const std::vector<Token>& tokens,const std::string& operator_str)
		{
			if (tokens.empty()) {
				return IsPrefixOperator(operator_str) ? UnaryType::Prefix : UnaryType::None;
			}

			const Token& last = tokens.back();

			// 1. Check: can it be postfix?
			if (IsPostfixOperator(operator_str)) {
				bool last_is_expression =
					last.type == TokenType::IDENTIFIER ||
					last.type == TokenType::TYPE_MARKER ||
					last.type == TokenType::LITERAL ||
					last.type == TokenType::COMPILATION_LABEL ||
					last.value.strVal == ")" ||
					last.value.strVal == "]" ||
					last.value.strVal == "}";
				if (last_is_expression) {
					return UnaryType::Postfix;
				}
			}

			// 2. Check: can it be prefix
			if (IsPrefixOperator(operator_str)) {
				// Standard checking for another prefix operators?
				bool last_is_not_expression =
					last.type == TokenType::OPERATOR ||
					last.type == TokenType::COMPILATION_LABEL ||
					last.value.strVal == "(" ||
					last.value.strVal == "[" ||
					last.value.strVal == "{" ||
					last.value.strVal == "," ||
					last.value.strVal == ":" ||
					last.value.strVal == ";" ||
					last.type == TokenType::KEYWORD;

				if (last_is_not_expression) {
					return UnaryType::Prefix;
				}
			}

			return UnaryType::None;
		}


		Token Lexer::ProcessOperator(LexingState& lex_state,
			const std::string& text,
			std::vector<Token>& tokens)
		{
			// Build operator
			std::string operator_;

			for (; lex_state.current_index < text.size(); lex_state.current_index++) {
				char c = text[lex_state.current_index];

				if (IsOperatorChar(c)) {
					operator_.push_back(c);

					//  Check: can it be more long operator?
					if (lex_state.current_index + 1 < text.size()) {
						char next_c = text[lex_state.current_index + 1];
						if (IsOperatorChar(next_c)) {
							std::string potential_op = operator_ + next_c;
							// Check: does this operator exist in the tokens table?
							// (in any formе: prefix, postfix, binary)
							if (TokensInfoTable::Get().GetTokenType(potential_op) != TokenType::UNDEFINED ||
								TokensInfoTable::Get().GetTokenType(potential_op + "u") != TokenType::UNDEFINED ||
								TokensInfoTable::Get().GetTokenType("u" + potential_op) != TokenType::UNDEFINED) {
								// Continue building
								continue;
							}
							else //operators line: ***c
							{
								
							}
						}
					}
					lex_state.current_index++;
					break;
				}
				else {
					break;
				}
			}

			if (operator_.empty()) {
				// Its not operator
				return Token(TokenType::UNDEFINED, lex_state.declaring_place);
			}

			// Define type of unarity when handing over already built operator
			UnaryType unary_type = IsUnary(lex_state, text, tokens, operator_);

			// Build final version of operator
			std::string final_op = operator_;

			switch (unary_type) {
			case UnaryType::Prefix:
				// Check: Does exist operator with postfix 'u' in the tokens table?
				if (TokensInfoTable::Get().GetTokenType(operator_ + "u") != TokenType::UNDEFINED) {
					final_op = operator_ + "u";
				}
				break;

			case UnaryType::Postfix:
				// Check: Does exist operator with prefix 'u' in the tokens table?
				if (TokensInfoTable::Get().GetTokenType("u" + operator_) != TokenType::UNDEFINED) {
					final_op = "u" + operator_;
				}
				break;

			case UnaryType::None:
				// Bunary operator - dont touch
				break;
			}

			// Find in the tokens table
			TokenType token_type = TokensInfoTable::Get().GetTokenType(final_op);

			if (token_type == TokenType::UNDEFINED) {
				// Try to find operator without modifier
				token_type = TokensInfoTable::Get().GetTokenType(operator_);
				if (token_type != TokenType::UNDEFINED) {
					final_op = operator_;
				}
				else {
					// Unknown operator
					Diagnostics::InformationMessage message("Unknown operator: '" + operator_ + "'.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, lex_state.declaring_place);

					Diagnostics::Logger::Get().Print(message);
					return Token(TokenType::UNDEFINED, lex_state.declaring_place);
				}
			}

			return Token(final_op, token_type, lex_state.declaring_place);
		}

		Token Lexer::InsertOpEnd(LexingState& lex_state, std::vector<Token>& tokens, const std::string& text)
		{
			if (tokens.size() == 0)return Token(TokenType::UNDEFINED, lex_state.declaring_place);
			if (text[lex_state.current_index] == ';') return Token((uint64_t)CompilationLabel::OPERATION_END, TokenType::COMPILATION_LABEL, lex_state.declaring_place);

			if (tokens.back().type == TokenType::COMPILATION_LABEL || tokens.back().value.strVal == "{" && lex_state.undefined_token.empty()) return Token(TokenType::UNDEFINED,lex_state.declaring_place);

			if (tokens.back().type != TokenType::COMPILATION_LABEL && tokens.back().value.strVal != "}" && text[lex_state.current_index] == '}')
			{
				return Token((uint64_t)CompilationLabel::OPERATION_END, TokenType::COMPILATION_LABEL, lex_state.declaring_place);
			}

			if (text[lex_state.current_index] == '\n' && !(GetNearCharWithoutSpaces(text, lex_state.current_index, 1) == '{' || GetNearCharWithoutSpaces(text, lex_state.current_index, -1) == '}'))
			{
				return Token((uint64_t)CompilationLabel::OPERATION_END, TokenType::COMPILATION_LABEL, lex_state.declaring_place);
			}

			return Token(TokenType::UNDEFINED, lex_state.declaring_place);
		}
		
		std::vector<Token> Lexer::ToTokens(std::string text, Definitions::ModuleId module_id)
		{
	
			LexingState state;
			state.declaring_place.module_id = module_id;
			bool in_string = false;
			state.tokens.reserve(text.size() / 4);
			bool in_symbol_literal = false;

			for (state.current_index = 0; state.current_index < text.size(); state.current_index++)
			{
				char c = text[state.current_index];
				if (c == '\r') continue;
				if (c == '\n')
				{
					state.declaring_place.place++;
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
						if (!state.was_comment) Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Excepts \"*/\".", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.declaring_place));
						continue;
					}

					if (c == ' ' || c == '\n' || c == '\t')
					{
						if (!state.undefined_token.empty())
						{
							state.tokens.push_back(CreateToken(state,state.undefined_token));
						}
						state.undefined_token.clear();

					}

					auto t = InsertOpEnd(state, state.tokens, text);
					if (t.type != TokenType::UNDEFINED)
					{
						if (!state.undefined_token.empty())
						{
							state.tokens.push_back(CreateToken(state,state.undefined_token));
							state.undefined_token.clear();
						}
						state.tokens.push_back(t);
						if (c == ';')continue;
					}
					if (state.was_comment) state.was_comment = false;


					if (GetTokenType(state,std::string(1, c)) == TokenType::OPERATOR)
					{
						std::string operator_ = std::string({ c });
						if (c == '.' && (state.undefined_token.size() == 0 || Strings::StringOperations::IsNumber(state.undefined_token))) //.1 = 0.1
						{
							state.undefined_token += c;
							continue;
						}
						if (!state.undefined_token.empty())	//Сразу добавляем токен перед ним, если есть
						{
							state.tokens.push_back(CreateToken(state,state.undefined_token));
							state.undefined_token.clear();
						}

						if (state.current_index < text.size() - 1 && GetTokenType(state,std::string({ c, text[state.current_index + 1] })) == TokenType::LITERAL && (state.tokens.size() == 0 || (state.tokens.back().type != TokenType::LITERAL && state.tokens.back().type != TokenType::IDENTIFIER)))  //if its number
						{
							state.was_neg_value = true;
							continue;
						}
						Token t = ProcessOperator(state,text, state.tokens);
						state.current_index--;
						if (t.type == TokenType::UNDEFINED)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid operator \"" + t.value.strVal + "\".", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.declaring_place));
							continue;
						}
						state.tokens.push_back(t);
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
							state.tokens.push_back(CreateToken(state,state.undefined_token));
							state.undefined_token.clear();
						}
						Token t;
						t.type = TokenType::DELIMITER;
						t.value = delimiter;
						t.declaring_place = state.declaring_place;
						if (c == '{')
						{
							state.current_depth++;
						}
						if (c == '}')
						{
							auto t = InsertOpEnd(state, state.tokens, text);
							if (t.type != TokenType::UNDEFINED)state.tokens.push_back(t);
							state.current_depth--;
						}

						state.tokens.push_back(t);
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
					state.tokens.push_back(CreateToken(state,state.undefined_token));
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
					state.tokens.push_back(CreateToken(state,state.undefined_token));
					state.undefined_token.clear();
					continue;
				}
				if ((c == ' ' || c == '\n' || c == '\t') && !in_string && !in_symbol_literal) continue;
				state.undefined_token.push_back(c);
			}

			if (!state.undefined_token.empty())
			{
				Token t;
				t.declaring_place = state.declaring_place;
				t.type = GetTokenType(state,state.undefined_token);
				t.value = GetTokenValue(state,state.undefined_token,t.type);
				state.tokens.push_back(t);
				state.tokens.push_back(Token((uint64_t)CompilationLabel::OPERATION_END, TokenType::COMPILATION_LABEL, state.declaring_place));
			}

			return state.tokens;
		}
	}
}