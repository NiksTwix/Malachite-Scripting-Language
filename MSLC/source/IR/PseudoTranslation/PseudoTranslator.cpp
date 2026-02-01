#include "..\..\..\include\IR\PseudoTranslation\PseudoTranslator.hpp"
#include  "..\..\..\include\IR\PseudoTranslation\PseudoDebugInfo.hpp"


namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			void PseudoTranslator::AnalyzeRecursive(AST::ASTNode& node, PseudoTranslationState& pts)
			{
				if (node.tokens.size() == 1 && node.tokens.front().type == Tokenization::TokenType::COMPILATION_LABEL) 
				{
					if (node.tokens.front().value.uintVal == (uint8_t)Tokenization::CompilationLabel::OPEN_VISIBLE_SCOPE) 
					{
						pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::PushFrame,0,0,0,node.tokens.front().line));
						pts.cs_observer->PushNewFrame();
					}
					else if (node.tokens.front().value.uintVal == (uint8_t)Tokenization::CompilationLabel::CLOSE_VISIBLE_SCOPE)
					{
						pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::PopFrame, 0, 0, 0, node.tokens.front().line));
						pts.cs_observer->PopFrame();
					}
					return;	//Compilation labels are simple tokens: they dont have children
				}
				size_t children_count = node.children.size();

				if (children_count && node.tokens.size())	//MSL syntax construction
				{
					//Basic constructions
				}
				else if (children_count && node.tokens.empty())				//Code block (without header)
				{
					for (auto child : node.children) 
					{
						AnalyzeRecursive(child,pts);
					}
				}
				else	//MSL Expression
				{
					expressions_translator.AnalyzeExpression(node.tokens,pts);
				}

			}
			POperationArray PseudoTranslator::AnalyzeTree(AST::ASTNode& root, CompilationInfo::CompilationState& cs)
			{
				PseudoTranslationState pts;
				pts.cs_observer = &cs;


				for (auto node : root.children) 
				{
					AnalyzeRecursive(node, pts);
				}
				CompilationStateStringSerializator csss;
				PseudoOperationsStringSerializator poss;
				std::cout << csss.Serialize(*pts.cs_observer);

				std::cout << poss.Serialize(pts.pseudo_code);


				return pts.pseudo_code;
			}
		}
	}
}