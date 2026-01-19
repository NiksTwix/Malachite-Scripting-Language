#include "..\..\..\include\IR\PseudoTranslation\ExpressionsTranslator.hpp"


namespace MSLC 
{
	namespace IntermediateRepresentation 
	{
		namespace Pseudo
		{

			void print_recursive(TokensGroup& gp, int depth)
			{
				//for (int i = 0; i < depth; i++) std::cout << "#";

				if (gp.IsSimple())std::cout << gp.simple.value.ToString() << "\n";
				else
				{
					if (gp.simple.type != TokenType::UNDEFINED) std::cout << gp.simple.value.ToString() << "\n";
					depth += 1;
					std::cout << "(" << '\n';
					for (auto child : gp.complex)
					{
						print_recursive(child, depth);
					}
					std::cout << ")" << '\n';
				}
			}


			Definitions::ChunkArray<PseudoOperation> ExpressionsTranslator::AnalyzeExpression(std::vector<Tokenization::Token> tokens, PseudoTranslationState& pts)
			{
				auto p = postfix_builder.BuildPostfix(tokens);
				int depth = 0;
				print_recursive(p, depth);

				return Definitions::ChunkArray<PseudoOperation>();
			}
		}
	}
}