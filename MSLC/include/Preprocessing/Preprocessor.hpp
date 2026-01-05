#pragma once
#include "..\CompilationInfo\CompilationState.hpp"
#include "..\Tokenization\TokensInfo.hpp"
#include "..\Definitions\ValueContainer.hpp"

namespace MSLC 
{
	namespace Preprocessing 
	{
		struct PreprocessingState
		{
			CompilationInfo::CompilationState* c_state; //observer
			size_t current_index;
			std::vector<Token>* tokens;//observer
			size_t current_line;
		};


		class Preprocessor 
		{
			void HandleDirective(PreprocessingState& state);

			void HandleDirectiveCall(PreprocessingState& state);



			//Directives handlers
			void HandleDefineConstant(PreprocessingState& state);
		public:
			void Preprocess(CompilationInfo::CompilationState& c_state, std::vector<Token>& tokens);
		};
	}

}