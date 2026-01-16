#include "..\..\..\include\IR\PseudoTranslation\PseudoTranslator.hpp"



namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			void PseudoTranslator::AnalyzeRecursive(AST::ASTNode& node, PseudoTranslationState& pts)
			{
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