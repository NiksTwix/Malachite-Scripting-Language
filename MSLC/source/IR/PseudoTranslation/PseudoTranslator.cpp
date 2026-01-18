#include "..\..\..\include\IR\PseudoTranslation\PseudoTranslator.hpp"



namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			void PseudoTranslator::AnalyzeRecursive(AST::ASTNode& node, PseudoTranslationState& pts)
			{
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
					Definitions::ChunkArray<PseudoOperation> operations = expressions_translator.AnalyzeExpression(node.tokens);
					pts.pseudo_code.OverwriteBack(operations);
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


				return pts.pseudo_code;
			}
		}
	}
}