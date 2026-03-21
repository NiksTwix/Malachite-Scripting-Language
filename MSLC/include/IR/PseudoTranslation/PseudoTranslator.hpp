#pragma once
#include "ExpressionsTranslator.hpp"
#include "BasicSyntaxTranslator.hpp"
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
				BasicSyntaxTranslator basic_syntax_translator;
			public:
				PseudoTranslator() = default;
				std::shared_ptr<PseudoTranslationState> AnalyzeTree(AST::ASTNode& root, CompilationInfo::CompilationState& cs);

			};
		}
	}
}