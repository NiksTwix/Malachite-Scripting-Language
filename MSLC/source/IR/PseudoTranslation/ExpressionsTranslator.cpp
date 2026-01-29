#include "..\..\..\include\IR\PseudoTranslation\ExpressionsTranslator.hpp"


namespace MSLC 
{
	namespace IntermediateRepresentation 
	{
		namespace Pseudo
		{

			void print_recursive(TokensGroup& gp, int depth)
			{
				//for (int i = 0; i < depth; i++) std::cout << "#";

				if (gp.IsSimple())std::cout << gp.simple.value.ToString() << "\n";
				else
				{
					if (gp.simple.type != TokenType::UNDEFINED) std::cout << gp.simple.value.ToString() << "\n";
					for (auto child : gp.complex)
					{
						print_recursive(child, depth);
					}
				}
			}


			void ExpressionsTranslator::AnalyzeAPST(TokensGroup& node, PseudoTranslationState& pts)
			{
				if (node.IsSimpleOrQN()) {
					//Node handling

				}
				
				//for (size_t i = 0; i < node.complex.size(); i++) {
				//	TokensGroup& group = node.complex[i];
				//	AnalyzeAPST(group,pts);
				//}

				switch (node.type)
				{
				case GroupType::Empty:
					break;
				case GroupType::Root:
					break;
				case GroupType::FunctionCall:
					break; 
				case GroupType::DataAccess:
					break;
				case GroupType::DataAccessChain:
					break;
				case GroupType::AttributeUsing:
					break;
				case GroupType::Argument:
					break;
				case GroupType::Operation:
					break;
				case GroupType::Declaration:

					break;
				case GroupType::Type:
					break;
				case GroupType::TypeCast:
					break;
				case GroupType::NewExpression:
					break;
				default:
					break;
				}
			}

			void ExpressionsTranslator::HandleSimple(TokensGroup& node, PseudoTranslationState& pts)
			{
				switch (node.type)
				{
				case GroupType::Simple:
					if (node.simple.type == TokenType::IDENTIFIER) {
						auto symbol = pts.cs_observer->FindSymbolLocal(node.simple.value.strVal);
						if (!symbol) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined identifier \"" + node.simple.value.ToString() + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.line));
							return;
						}
						pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::Use, symbol->description_id,0,0, node.line));
					}
					else if (node.simple.type == TokenType::LITERAL) 
					{
						auto constant = pts.cs_observer->GetICT().GetOrAdd(node.simple.value);
						pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::UseConstant, constant, 0, 0, node.line));
					}
					else 
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined simple tokens group \"" + node.simple.value.ToString() + "\".", Diagnostics::DeveloperError, Diagnostics::SourceCode, node.line));
					}
					break;
				case GroupType::ArrayLiteral:
					for (auto arg : node.complex)
					{
						AnalyzeAPST(arg, pts);	
					}
					pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::CreateArray, node.complex.size(), 0, 0, node.line));
					break;
				case GroupType::QualifiedName:
				{
					CompilationInfo::Symbol* current = nullptr;
					for (size_t i = 0; i < node.complex.size(); i++) 
					{
						auto& identifier = node.complex[i];
						
						if (identifier.simple.type != TokenType::IDENTIFIER) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid identifier \"" + identifier.simple.value.ToString() + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.line));
							return;
						}
						auto str_val = identifier.simple.value.ToString();
						if ( i == 0) current = pts.cs_observer->FindSymbolLocal(str_val);
						else 
						{
							if (current->type == CompilationInfo::SymbolType::Namespace)
							{
								auto frame = pts.cs_observer->GetGST().GetNamespace(current->description_id);
								if (!frame->lsl.Has(str_val)) {
									Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined identifier \"" + str_val + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.line));
									return;
								}
								current = &frame->lsl.Get(str_val);
							}
							else if (current->type == CompilationInfo::SymbolType::Type) 
							{
								//Access to static member. TODO Checking on function, fields and etc!
							}
							if (i == node.complex.size() - 1) 
							{
								pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::Use, current->description_id, 0, 0, node.line));
								return;
							}
						}
						if (current == nullptr) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined identifier \"" + str_val + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.line));
							return;
						}
					}
				}
					break;
				}
			}

			Definitions::ChunkArray<PseudoOperation> ExpressionsTranslator::AnalyzeExpression(std::vector<Tokenization::Token> tokens, PseudoTranslationState& pts)
			{
				auto p = postfix_builder.Build(tokens);
				int depth = 0;
				print_recursive(p, depth);
				//print_recursive(p, depth);
				auto p1 = apst_builder.BuildAPST(p);
				
				
				AnalyzeAPST(p1, pts);
				return pts.pseudo_code;
			}
		}
	}
} 