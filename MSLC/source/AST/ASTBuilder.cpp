#include "..\..\include\AST\ASTBuilder.hpp"


namespace MSLC 
{
	namespace AST
	{
		void ASTBuilder::if_scope_exception(Tokenization::Token& t, ASTState& state)
		{
			if (scope_exceptions.count(t.value.strVal))
			{
				state.skip_scopes.push({ true,state.current_depth });
			}
		}
		ASTNodeType ASTBuilder::DefineType(std::vector<Tokenization::Token>& t)
		{
			static const std::unordered_map<std::string, ASTNodeType> keywordMap = {
				{std::string(Keywords::w_for), ASTNodeType::ForCycle},
				{std::string(Keywords::w_while), ASTNodeType::WhileCycle},
				{std::string(Keywords::w_if), ASTNodeType::If},
				{std::string(Keywords::w_elif), ASTNodeType::Elif},
				{std::string(Keywords::w_else), ASTNodeType::Else},
				{std::string(Keywords::w_namespace), ASTNodeType::Namespace},
				{std::string(Keywords::w_func), ASTNodeType::Function},
				{std::string(Keywords::w_object_template), ASTNodeType::ObjectTemplate},
			};
			ASTNodeType foundType = ASTNodeType::Expression;
			int keywordCount = 0;

			for (auto& token : t)
			{
				if (token.type != Tokenization::TokenType::KEYWORD) continue;

				auto it = keywordMap.find(token.value.strVal);
				if (it != keywordMap.end()) {
					keywordCount++;
					if (keywordCount > 1) {
						// Found much than 1 keyword (construction marker/name) - its error
						return ASTNodeType::None;
					}
					foundType = it->second;
				}
			}

			return foundType;
		}
		

		ASTNode ASTBuilder::BuildAbsractScopeTree(std::vector<Tokenization::Token>& tokens)
		{
			ASTNode root;
			root.type = ASTNodeType::CodeBlock;
			std::stack<ASTNode> cn_stack;

			cn_stack.push(root);

			std::vector<Tokenization::Token> command_tokens;

			ASTState state;

			auto line_print = [](std::vector<Tokenization::Token>& tokens) -> std::string
				{
					std::string constructor;
					for (Tokenization::Token& token : tokens) 
					{
						constructor += token.value.ToString();
						constructor += " ";
					}
					if (!constructor.empty()) constructor.pop_back();	//deleting of extra space
					return constructor;
				};


			for (int i = 0; i < tokens.size(); i++)
			{
				Tokenization::Token& t = tokens[i];
				state.last_line = t.line;


				if (t.value.strVal == "{")
				{
					if (command_tokens.empty()) 
					{
						ASTNode empty_node;
						empty_node.type = ASTNodeType::CodeBlock;
						empty_node.line = t.line;
						cn_stack.push(empty_node);
						ASTNode scope_start;
						scope_start.tokens.push_back(Tokenization::Token((uint64_t)Tokenization::CompilationLabel::OPEN_VISIBLE_SCOPE, Tokenization::TokenType::COMPILATION_LABEL, t.line, t.module_id));
						cn_stack.top().children.push_back(scope_start);
					}
					else 
					{
						//Define type of header
						ASTNodeType type = DefineType(command_tokens);
						if (type == ASTNodeType::None) 
						{
							Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Incorrect code structure: " + line_print(command_tokens) + ".", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, t.line));
							continue;
						}
						if (type != ASTNodeType::Expression)	//Multyline block
						{
							ASTNode node;
							node.line = command_tokens.back().line;
							node.tokens = std::move(command_tokens);
							node.type = type;
							cn_stack.push(node);
							ASTNode scope_start;
							scope_start.tokens.push_back(Tokenization::Token((uint64_t)Tokenization::CompilationLabel::OPEN_VISIBLE_SCOPE, Tokenization::TokenType::COMPILATION_LABEL, t.line, t.module_id));
							cn_stack.top().children.push_back(scope_start);
						}
						else //Expression -> code block
						{
							//Expression
							ASTNode prev_node;
							prev_node.line = command_tokens.back().line;
							prev_node.type = ASTNodeType::Expression;
							prev_node.tokens = std::move(command_tokens);
							cn_stack.top().children.push_back(prev_node);

							//Code block
							ASTNode empty_node;
							empty_node.type = ASTNodeType::CodeBlock;
							empty_node.line = t.line;
							cn_stack.push(empty_node);
							ASTNode scope_start;
							scope_start.tokens.push_back(Tokenization::Token((uint64_t)Tokenization::CompilationLabel::OPEN_VISIBLE_SCOPE, Tokenization::TokenType::COMPILATION_LABEL, t.line, t.module_id));
							cn_stack.top().children.push_back(scope_start);
						}
					}
					
					state.current_depth++;
					continue;
				}
				if (t.value.strVal == "}")
				{
					if (cn_stack.size() <= 1)
					{
						Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("There are extra ones '}'.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, t.line));
						continue;
					}

					ASTNode scope_end;
					scope_end.tokens.push_back(Tokenization::Token((uint64_t)Tokenization::CompilationLabel::CLOSE_VISIBLE_SCOPE, Tokenization::TokenType::COMPILATION_LABEL, t.line, t.module_id));	//TODO инрементировать, если надо
					cn_stack.top().children.push_back(scope_end);

					auto node = cn_stack.top();
					cn_stack.pop();
					cn_stack.top().children.push_back(node);
					state.current_depth--;
					continue;
				}
				if (t.type == Tokenization::TokenType::COMPILATION_LABEL && t.value.uintVal == (uint64_t)Tokenization::CompilationLabel::OPERATION_END)
				{
					if (command_tokens.size() != 0)
					{
						ASTNode node;
						command_tokens.push_back(t);
						node.line = command_tokens.back().line;
						node.type = DefineType(command_tokens);
						node.tokens = std::move(command_tokens);
						cn_stack.top().children.push_back(node);
					}
					continue;
				}

				if (state.current_depth < 0)
				{
					Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("There are extra ones '}'.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, t.line));
					continue;
				}
				command_tokens.push_back(t);
			}

			if (!command_tokens.empty())
			{
				ASTNode node;
				node.line = command_tokens.back().line;
				node.type = DefineType(command_tokens);
				node.tokens = std::move(command_tokens);
				cn_stack.top().children.push_back(node);
			}

			if (state.current_depth > 0)
			{
				Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Expected '}'.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, state.last_line));
			}

			if (cn_stack.empty()) 
			{
				Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Developer Message:AST Stack underflow.", Diagnostics::MessageType::LogicError, Diagnostics::SourceType::SourceCode, state.last_line));
				return root;
			}
			root = cn_stack.top();
			return root;
		}

		
	}
}