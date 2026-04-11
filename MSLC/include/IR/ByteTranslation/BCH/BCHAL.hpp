#pragma once
#include "BCHUtils.hpp"

namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			namespace BCH 
			{
				class ArithmeticLogicHandler 
				{
				public:
					void Handle(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
				};
			}
			
		}
	}
}
