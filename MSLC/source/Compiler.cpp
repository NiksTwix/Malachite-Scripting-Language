#include "..\include\Compiler.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace MSLC 
{
	void Compiler::TestCompile(std::string path)
	{
		fs::path path_ = fs::path(path);

		if (!path_.has_filename())
		{
			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid path to file."));
			return;
		}

		fs::path dpath = path_.parent_path();
		fs::path stem = path_.stem();

		fs::path result_dir = dpath / stem;

		std::filesystem::create_directory(result_dir);

		
		Preprocessing::HandlingOrder handling_order = amg_builder.Analyze(path);
		CompilationInfo::CompilationState comp_state;

		

		for (auto id : handling_order.processing_order) 
		{
			comp_state.SetModule(id);
			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Compiling is started: \"" + handling_order.files[id].path + "\"."));

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Loading text of file."));

			std::string text = amg_builder.LoadFile(handling_order.files[id].path).str();

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Lexing."));

			auto tokens = lexer.ToTokens(text);

			

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Preprocessing 1."));

			preprocessor.Preprocess(comp_state, tokens);

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: AST building."));

			AST::ASTNode tree = ast_builder.BuildAbsractScopeTree(tokens);
			ast_builder.Postprocess(tree);

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Pseudo building."));
			auto pseudo_state = preudo_translator.AnalyzeTree(tree, comp_state);


			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Byte building."));

			auto b_state = byte_translator.Translate(pseudo_state, &comp_state,IntermediateRepresentation::Byte::ByteTranslationConfig());

			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Stage: Packing."));

			auto bytes = packer.ByterizeCommands(&comp_state, b_state, id);



			bool result = packer.SaveAsMSLCO(result_dir, bytes, id);
			delete bytes.first;
			if (!result) 
			{
				Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Saving in object file has failed."));
				continue;
			}
				
			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("File with code has compiled. Object file has saved on path: \"" + result_dir.string() + "\""));
		}

		auto bytes = packer.ByterizeCompilationState(&comp_state, handling_order);
		bool result = packer.SaveAsMSLMO(result_dir, bytes, stem.string());
		if (!result)
		{
			Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Saving compilation info in object file has failed."));
		}
		Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("File with compilation info has compiled. Object file has saved on path: \"" + result_dir.string() + "\""));
		delete bytes.first;
	}
}