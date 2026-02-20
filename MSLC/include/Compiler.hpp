#pragma once
#include "Preprocessing/AMGBuilder.hpp"
#include "Preprocessing/Preprocessor.hpp"
#include "Tokenization/Lexer.hpp"
#include "AST/ASTBuilder.hpp"
#include "IR/PseudoTranslation/PseudoTranslator.hpp"
#include "IR/ByteTranslation/ByteTranslator.hpp"
#include "Packing/Packer.hpp"

namespace MSLC 
{

	class Compiler
	{
	private:
		Tokenization::Lexer lexer;
		Preprocessing::AMGBuilder amg_builder;
		Preprocessing::Preprocessor preprocessor;
		AST::ASTBuilder ast_builder;
		IntermediateRepresentation::Pseudo::PseudoTranslator preudo_translator;
		IntermediateRepresentation::Byte::ByteTranslator byte_translator;
		Packing::Packer packer;
	public:

		void TestCompile(std::string path);	//test compiling for development

	};


}