#pragma once
#include "BCH\BCHAL.hpp"
#include "BCH\BCHDataFlow.hpp"
#include "BCH\BCHControlFlow.hpp"
#include "ByteLLTranslator.hpp"
namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			class CommandsHandler
			{
			private:
				BCH::ArithmeticLogicHandler alh;
				BCH::DataFlowHandler dfh;
				BCH::ControlFlowHandler cfh;
				::MSLC::IntermediateRepresentation::Byte::LLTranslator ll_translator;
			public:
				void HandleCommand(std::shared_ptr<ByteTranslationState>  b_state);

				void FreeTempResources(std::shared_ptr<ByteTranslationState> b_state);

			};
			

		}
	}
}