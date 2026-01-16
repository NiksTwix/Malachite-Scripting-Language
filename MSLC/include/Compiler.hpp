#pragma once
#include "Preprocessing/AMGBuilder.hpp"
#include "Preprocessing/Preprocessor.hpp"
#include "Tokenization/Lexer.hpp"
#include "AST/ASTBuilder.hpp"
#include "IR/PseudoTranslation/PseudoTranslator.hpp"

namespace MSLC 
{

	class Compiler
	{
	private:
		Tokenization::Lexer lexer;
		Preprocessing::AMGBuilder amg_builder;
		Preprocessing::Preprocessor preprocessor;
		AST::ASTBuilder ast_builder;
	public:

		void TestCompile(std::string path);	//test compiling for development

	};


}