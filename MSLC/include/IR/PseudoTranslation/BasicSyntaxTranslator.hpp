#pragma once
#include "PseudoTranslationsInfo.hpp"



namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			class BasicSyntaxTranslator 
			{
			public:
				BasicSyntaxTranslator() = default;
				void HandleBasicSyntax(AST::ASTNode& node, PseudoTranslationState& pts, recursive_handler rhandler);
			};
		}
	}
}