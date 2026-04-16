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
				class ControlFlowHandler {

				public:
					void Handle(Pseudo::POperationArray& p_array, std::shared_ptr<ByteTranslationState>  b_state);
				};
			}
		}
	}
}