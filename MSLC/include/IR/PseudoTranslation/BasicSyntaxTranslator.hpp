#pragma once
#include "LowLevelCode\PseudoLLTranslator.hpp"



namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			class BasicSyntaxTranslator 
			{
				LLTranslator lltranslator;

			public:
				BasicSyntaxTranslator() = default;
				void HandleBasicSyntax(AST::ASTNode& node, PseudoTranslationState& pts, recursive_handler rhandler);
			};
		}
	}
}