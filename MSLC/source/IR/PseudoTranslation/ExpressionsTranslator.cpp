#include "..\..\..\include\IR\PseudoTranslation\ExpressionsTranslator.hpp"


namespace MSLC 
{
	namespace IntermediateRepresentation 
	{
		namespace Pseudo
		{
			Definitions::ChunkArray<PseudoOperation> ExpressionsTranslator::AnalyzeExpression(std::vector<Tokenization::Token> tokens, PseudoTranslationState& pts)
			{
				return Definitions::ChunkArray<PseudoOperation>();
			}
		}
	}
}