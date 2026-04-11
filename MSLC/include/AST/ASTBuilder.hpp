#pragma once
#include "..\Tokenization\TokensInfo.hpp"
#include "..\Definitions\ValueContainer.hpp"
#include  <unordered_set>
#include <stack>

namespace MSLC 
{
	namespace AST
	{

		enum class ASTNodeType
		{
			None,
			Expression,
			CodeBlock,		//{} unnamed visible scope
			Namespace,
			If,
			Elif,
			Else,
			ForCycle,
			WhileCycle,
			Function,
			ObjectTemplate,		//Class/struct analog
			LowLevelCodeBlock,		//op_code
			NodeGroup
		};

		enum class ASTGroupType 
		{
			None,
			IfCondition,
		};



		struct ASTNode
		{
			std::vector<Tokenization::Token> tokens;
			std::vector<ASTNode> children;
			Diagnostics::DeclaringPlace declaring_place;
			ASTNodeType type  = ASTNodeType::None;
			ASTGroupType group_type = ASTGroupType::None;
		};


		class ASTBuilder	//AbstractScopeTree
		{
		private:
			struct SkipScope {
				bool flag = false;
				int depth = -1;
			};
			struct ASTState
			{
				int current_depth = 0;
				Diagnostics::DeclaringPlace last_declaring_place;
				std::stack<SkipScope> skip_scopes;
			};


			std::unordered_set<std::string> scope_exceptions = {
				//std::string(Keywords::w_func),
				//std::string(Keywords::w_op_code),
				//std::string(Keywords::w_object_template),
				//std::string(Keywords::w_alias),
				//std::string(Keywords::w_for) 
			};	//ќбъ€влени€, в блоках кода которых не надо ставить автоматический SCOPE 
			void if_scope_exception(Tokenization::Token& t, ASTState&  ast_state);

			//ASTNode UniteToGroup(std::vector<ASTNode>& nodes, CompilationLabel label);

			ASTNodeType DefineType(std::vector<Tokenization::Token>& t);


			//Postprocessing
			std::unordered_set<ASTNodeType> start_of_group_nodes = { ASTNodeType::If };	//Markers of group's start, edge cases

			void UnitToGroup(ASTNode& parent, std::vector<std::pair<ASTNode, size_t>>& group, ASTGroupType group_type);
			ASTGroupType GetGroupType(ASTNode& node);

		public:

			ASTNode BuildAbsractScopeTree(std::vector<Tokenization::Token>& tokens);


			void Postprocess(ASTNode& root);

		};
	}
}