#include "..\..\..\include\IR\PseudoTranslation\BasicSyntaxTranslator.hpp"
namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			void BasicSyntaxTranslator::HandleIfCondition(AST::ASTNode& node, PseudoTranslationState& pts, recursive_handler rhandler)
			{

				bool was_else = false;

				bool use_end_label = node.children.size() > 1;

				CompilationInfo::LabelID end_label = use_end_label ? pts.cs_observer->GetGST().GetNewLabelID() : CompilationInfo::INVALID_ID;

				//if condition structure checking: if, elif, elif..., else

				if (node.children.front().type != AST::ASTNodeType::If) 
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block starts not from 'if'.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
					return;
				}
				
				for (size_t index = 1; index < node.children.size(); index++) 
				{
					AST::ASTNode& child = node.children[index];
					if (child.type == AST::ASTNodeType::Else)
					{
						was_else = true; continue;
					}
					if ((child.type == AST::ASTNodeType::Elif || child.type == AST::ASTNodeType::Else) && was_else) 
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block is invalid because of elif and else statements are after last else statement.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
						return;
					}
				}



				for (AST::ASTNode& child : node.children) 
				{

					
					if (child.type == AST::ASTNodeType::If || child.type == AST::ASTNodeType::Elif)
					{

						//Condition's parsing

						//[if_keyword] (expression) -> statement
						//[if_keyword] (expression) { statement }

						if (child.tokens.size() < 4)	//if (true)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. If/elif block is without arguments.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
							return;
						}
						if (const auto& parenthese = child.tokens[1]; parenthese.type != TokenType::DELIMITER || parenthese.value.strVal != "(")	//if (true)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. Condition's expression must be enclosed in parentheses.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
							return;
						}

						AST::ASTNode condition;
						condition.type = AST::ASTNodeType::Expression;

						AST::ASTNode line_statement;
						line_statement.type = AST::ASTNodeType::Expression;

						int parentheses_depth = 1;	// we have skiped open (
						size_t ci = 2;				//skip keyword and (

						bool build_condition = true;
						bool build_line_statement = false;	// expression -> statement
						for (; ci < child.tokens.size(); ci++)		// if ( expression[2]
						{
							const Token& t = child.tokens[ci];
							if (t.type == TokenType::DELIMITER && t.value.strVal == "(")
							{
								if (!build_condition) 
								{
									Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. '(' stands after condition.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
									return;
								}
								parentheses_depth++;
							}
							if (t.type == TokenType::DELIMITER && t.value.strVal == ")") 
							{
								parentheses_depth--;
								if (parentheses_depth == 0) 
								{
									build_condition = false;
								}
							}

							if (t.type == TokenType::OPERATOR && t.value.strVal == "->"	&& !build_condition) 
							{
								build_line_statement = true;

								line_statement.tokens = Strings::StringOperations::TrimVector(child.tokens, ci + 1, child.tokens.size() - 1);
								break;
							}

							if (build_condition) condition.tokens.push_back(t);
						}

						if (parentheses_depth > 0) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. Condition's expression must be enclosed in parentheses.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
							return;
						}
						if (parentheses_depth < 0) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. Extaneous ')' exist in condition's expression.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
							return;

						}
						if (condition.tokens.empty()) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. Condition's expression is null.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
							return;
						}
						if (build_line_statement && line_statement.tokens.empty())
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. Line statement ([if condition] -> line_statement) is null.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
							return;
						}


						rhandler(condition, pts);	// condition handling

						CompilationInfo::LabelID not_valid_jump_label = pts.cs_observer->GetGST().GetNewLabelID();

						PseudoOperation jump = PseudoOperation(PseudoOpCode::JumpNIf, not_valid_jump_label, child.declaring_place);
						pts.pseudo_code.Pushback(jump);

						if (build_line_statement) 
						{
							rhandler(line_statement, pts);	// line_statement handling
						}
						else 
						{
							for (auto body_line:child.children) rhandler(body_line, pts);	//WARNING! keep controlling visible scopes
						}

						if (use_end_label) 
						{
							jump = PseudoOperation(PseudoOpCode::Jump, end_label, child.declaring_place);
							pts.pseudo_code.Pushback(jump);
						}

						PseudoOperation label = PseudoOperation(PseudoOpCode::Label, not_valid_jump_label, child.declaring_place);
						pts.pseudo_code.Pushback(label);
					}

					else if (child.type == AST::ASTNodeType::Else)
					{
						// else -> 
						// else {}

						AST::ASTNode line_statement;
						line_statement.type = AST::ASTNodeType::Expression;


						if (child.tokens.size() > 1) 
						{
							if (const Token& t = child.tokens[1]; t.type != TokenType::OPERATOR || t.value.strVal != "->") 
							{
								Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[else] is invalid. Operator '->' is expected for line statetement defining.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
								return;
							}

							line_statement.tokens = Strings::StringOperations::TrimVector(child.tokens, 2, child.tokens.size() - 1);	//skip ->

							rhandler(line_statement, pts);

						}
						else 
						{
							for (auto body_line : child.children) rhandler(body_line, pts);	//WARNING! keep controlling visible scopes
						}

						//JMP command for skipping is not needed because else in the end of condition construction
					}
					
					else 
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Unknown structure exists in conditions block.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
						return;
					}

				}

				if (use_end_label) 
				{
					PseudoOperation endlabel = PseudoOperation(PseudoOpCode::Label, end_label, node.declaring_place);
					pts.pseudo_code.Pushback(endlabel);
				}
				
			}



			void BasicSyntaxTranslator::HandleBasicSyntax(AST::ASTNode& node, PseudoTranslationState& pts, recursive_handler rhandler)
			{

				switch (node.type)
				{
					case AST::ASTNodeType::None: break;
					case AST::ASTNodeType::Namespace: break;
					case AST::ASTNodeType::NodeGroup: 
					{
						switch (node.group_type)
						{
						case AST::ASTGroupType::IfCondition:
							HandleIfCondition(node, pts, rhandler);
							break;
						case AST::ASTGroupType::None:
							//In the future move it to separated handler

							break;
						default:
							break;
						}
					}
					case AST::ASTNodeType::ForCycle: break;
					case AST::ASTNodeType::WhileCycle: break;
					case AST::ASTNodeType::Function: break;
					case AST::ASTNodeType::ObjectTemplate: break;
					case AST::ASTNodeType::LowLevelCodeBlock:
						lltranslator.Translate(pts, node);
						break;
					case AST::ASTNodeType::CodeBlock:
					{
						for (auto child : node.children)
						{
							rhandler(child, pts);
						}

					}
				default:
					break;
				}
				


				
			}

			void BasicSyntaxTranslator::ClearTempResources()
			{
				lltranslator.ClearTempResources();
			}
		}
	}
}
