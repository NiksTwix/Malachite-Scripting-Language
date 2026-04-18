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


				void HandleIfCondition(AST::ASTNode& node, PseudoTranslationState& pts, recursive_handler rhandler);
				void HandleWhile(AST::ASTNode& node, PseudoTranslationState& pts, recursive_handler rhandler);
			public:
				BasicSyntaxTranslator() = default;
				void HandleBasicSyntax(AST::ASTNode& node, PseudoTranslationState& pts, recursive_handler rhandler);

				void ClearTempResources();

			};
		}
	}
}