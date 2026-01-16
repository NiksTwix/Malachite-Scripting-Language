#pragma once
#include "PseudoTranslationsInfo.hpp"
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

			public:

				POperationArray AnalyzeTree(AST::ASTNode& root, CompilationInfo::CompilationState& cs);

			};
		}
	}
}