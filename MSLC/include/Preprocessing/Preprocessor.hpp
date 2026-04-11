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


		struct MacroArgument
		{
			std::vector<Token> tokens;
		};

		class Preprocessor 
		{
			void HandleDirective(PreprocessingState& state);

			void HandleDirectiveCallImmediately(PreprocessingState& state);



			//Directives handlers
			void HandleDefineConstant(PreprocessingState& state);
			void HandleDefineInsertion(PreprocessingState& state);

			void Postprocess(CompilationInfo::CompilationState& c_state, std::vector<Token>& tokens);

			void InsertMacro(CompilationInfo::CompilationState& c_state, std::vector<Token>& tokens, std::vector<Token>& source, size_t& i, MacroDefinition* macro);

		public:
			void Preprocess(CompilationInfo::CompilationState& c_state, std::vector<Token>& tokens);
		};
	}

}