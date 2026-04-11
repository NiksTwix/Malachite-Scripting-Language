#pragma once
#include "..\PseudoTranslationsInfo.hpp"
#include <unordered_map>
namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			constexpr size_t MAX_VALUE_SIZE = 8;

			class LLTranslator 
			{
				struct Argument
				{
					std::vector<Token> tokens;
					bool valid = false;
				};

				

				void Handle(PseudoTranslationState& state, std::vector<Argument>& arguments, std::vector<Token>& op_code, Diagnostics::DeclaringPlace  di);
			public:

				

				void Translate(PseudoTranslationState& state, const AST::ASTNode& node);

			};
		}
	}
}