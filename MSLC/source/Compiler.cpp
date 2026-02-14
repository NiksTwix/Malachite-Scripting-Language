#include "..\include\Compiler.hpp"

namespace MSLC 
{
	void Compiler::TestCompile(std::string path)
	{

		Preprocessing::HandlingOrder handling_order = amg_builder.Analyze(path);

		for (auto id : handling_order.processing_order) 
		{
			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Compiling is started: \"" + handling_order.files[id].path + "\"."));

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Loading text of file."));

			std::string text = amg_builder.LoadFile(handling_order.files[id].path).str();

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Lexing."));

			auto tokens = lexer.ToTokens(text);

			CompilationInfo::CompilationState comp_state;

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Preprocessing 1."));

			preprocessor.Preprocess(comp_state, tokens);

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: AST building."));

			AST::ASTNode tree = ast_builder.BuildAbsractScopeTree(tokens);
			ast_builder.Postprocess(tree);

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Pseudo building."));
			auto parray = preudo_translator.AnalyzeTree(tree, comp_state);


			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Byte building."));

			auto b_state = byte_translator.Translate(parray, &comp_state,IntermediateRepresentation::Byte::ByteTranslationConfig());


		}

	}
}