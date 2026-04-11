#pragma once
#include "BCHAL.hpp"
namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			namespace BCH
			{
				class DataFlowHandler {
				
				public:
					void HandleMemory(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state, ArithmeticLogicHandler& alh);

					void HandleUsing(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);

					void HandleDeclaring(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
				};
			}
		}
	}
}