#include "..\include\Compiler.hpp"

namespace MSLC 
{
	void Compiler::TestCompile(std::string path)
	{

		Preprocessing::HandlingOrder handling_order = amg_builder.Analyze(path);

		for (auto id : handling_order.processing_order) 
		{
			Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Compiling is started: \"" + handling_order.files[id].path + "\"."));

			Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Stage: Loading text of file."));

			std::string text = amg_builder.LoadFile(handling_order.files[id].path).str();

			Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Stage: Lexing."));

			auto tokens = lexer.ToTokens(text);

			CompilationInfo::CompilationState comp_state;

			Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Stage: Preprocessing 1."));

			preprocessor.Preprocess(comp_state, tokens);

			Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Stage: AST Building."));

			AST::ASTNode tree = ast_builder.BuildAST(tokens);
		}

	}
}