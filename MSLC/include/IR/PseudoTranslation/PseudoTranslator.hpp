#pragma once
#include "ExpressionsTranslator.hpp"
#include "..\..\AST\ASTBuilder.hpp"


namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{

			class PseudoTranslator 
			{
			private:

				void AnalyzeRecursive(AST::ASTNode& node, PseudoTranslationState& pts);

				ExpressionsTranslator expressions_translator;

			public:
				PseudoTranslator() = default;
				POperationArray AnalyzeTree(AST::ASTNode& root, CompilationInfo::CompilationState& cs);

			};
		}
	}
}