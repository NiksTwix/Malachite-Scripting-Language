#pragma once
#include "..\Tokenization\TokensInfo.hpp"
#include "..\Definitions\ValueContainer.hpp"
#include  <unordered_set>
#include <stack>

namespace MSLC 
{
	namespace AST
	{
		struct ASTNode
		{
			std::vector<Tokenization::Token> tokens;
			std::vector<ASTNode> children;
			int line;
		};

		

		class ASTBuilder
		{
		private:
			struct SkipScope {
				bool flag = false;
				int depth = -1;
			};
			struct ASTState
			{
				int current_depth = 0;
				int last_line = 0;
				std::stack<SkipScope> skip_scopes;
			};
			std::unordered_set<std::string> scope_exceptions = {
				std::string(Keywords::w_func),
				std::string(Keywords::w_op_code),
				std::string(Keywords::w_object_template),
				std::string(Keywords::w_alias),
				std::string(Keywords::w_for) };	//ќбъ€влени€, в блоках кода которых не надо ставить автоматический SCOPE 
			void if_scope_exception(Tokenization::Token& t, ASTState&  ast_state);

			//ASTNode UniteToGroup(std::vector<ASTNode>& nodes, CompilationLabel label);



		public:
			ASTNode BuildAST(std::vector<Tokenization::Token>& tokens);
			//void PostprocessTree(ASTNode& node);
		};
	}
}