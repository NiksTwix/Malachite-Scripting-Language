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

				AST::ASTNode insert_after_end_jump;


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
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. If/elif block is without arguments.", Diagnostics::SyntaxError, Diagnostics::SourceCode, child.declaring_place));
							return;
						}
						if (const auto& parenthese = child.tokens[1]; parenthese.type != TokenType::DELIMITER || parenthese.value.strVal != "(")	//if (true)
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. Condition's expression must be enclosed in parentheses.", Diagnostics::SyntaxError, Diagnostics::SourceCode, child.declaring_place));
							return;
						}

						AST::ASTNode condition;
						condition.type = AST::ASTNodeType::Expression;

						AST::ASTNode line_statement;
						line_statement.type = AST::ASTNodeType::Expression;

						int parentheses_depth = 1;	// we have skiped open (
						size_t ci = 1;				//skip keyword

						bool build_condition = true;
						for (; ci < child.tokens.size(); ci++)		// if ( expression[2]
						{
							const Token& t = child.tokens[ci];
							if (t.type == TokenType::DELIMITER && t.value.strVal == "(" && build_condition)
							{
								if (parentheses_depth == 0) 
								{
									parentheses_depth++; continue;
								}
								parentheses_depth++;
							}
							if (t.type == TokenType::DELIMITER && t.value.strVal == ")" && build_condition)
							{
								parentheses_depth--;
								if (parentheses_depth == 0) 
								{
									build_condition = false;
									continue;
								}
							}
							if (build_condition) condition.tokens.push_back(t);
							else 
							{
								line_statement.tokens.push_back(t);
							}
						}

						if (condition.tokens.empty()) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Conditions block[if/elif] is invalid. Condition's expression is null.", Diagnostics::SyntaxError, Diagnostics::SourceCode, child.declaring_place));
							return;
						}
						rhandler(condition, pts);	// condition handling

						CompilationInfo::LabelID not_valid_jump_label = pts.cs_observer->GetGST().GetNewLabelID();

						PseudoOperation jump = PseudoOperation(PseudoOpCode::JumpNIf, not_valid_jump_label, child.declaring_place);
						pts.pseudo_code.Pushback(jump);

						if (!line_statement.tokens.empty()) 
						{
							rhandler(line_statement, pts);	// line_statement handling
						}
						else
						{
							for (auto body_line : child.children) rhandler(body_line, pts);	//WARNING! keep controlling visible scopes
						}

						if (use_end_label) 
						{
							jump = PseudoOperation(PseudoOpCode::Jump, end_label, child.declaring_place);
							pts.pseudo_code.Pushback(jump);
						}

						PseudoOperation label = PseudoOperation(PseudoOpCode::Label, not_valid_jump_label, child.declaring_place);
						pts.pseudo_code.Pushback(label);


						if (node.children.size() > 1 && !line_statement.tokens.empty() && !child.children.empty()) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Breaking the structure of the condition block.", Diagnostics::SyntaxError, Diagnostics::SourceCode, child.declaring_place));
							return;
						}

						if (!line_statement.tokens.empty()) //Insert code_block after line_statement and jump (if this block is last)
						{
							insert_after_end_jump = child;	
						}
					}

					else if (child.type == AST::ASTNodeType::Else)
					{
						// else statement
						// else {}
						AST::ASTNode line_statement;
						line_statement.type = AST::ASTNodeType::Expression;
						if (child.tokens.size() > 1) 
						{
							line_statement.tokens = Strings::StringOperations::TrimVector(child.tokens, 1, child.tokens.size() - 1);	//skip else

							rhandler(line_statement, pts);
							if (use_end_label && !child.children.empty()) 
							{
								insert_after_end_jump = child;
							}
						}
						else 
						{
							for (auto body_line : child.children) rhandler(body_line, pts);	//WARNING! keep controlling visible scopes
						}
						//JMP command for skipping is not needed because else in the end of condition construction
					}
					
					else 
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Unknown structure exists in conditions block.", Diagnostics::SyntaxError, Diagnostics::SourceCode, child.declaring_place));
						return;
					}

				}

				if (use_end_label) 
				{
					PseudoOperation endlabel = PseudoOperation(PseudoOpCode::Label, end_label, node.declaring_place);
					pts.pseudo_code.Pushback(endlabel);


					for (auto body_line: insert_after_end_jump.children) rhandler(body_line, pts);	//WARNING! keep controlling visible scopes

				}
				
				/*
				if (x>0) y = 1
				{ //Code block (it mustnt be connected with if)
				     z=2
				}
				It fixes a bug of AST builder. Because it[ASTBuilder] doesnt consider 'if's semantic we should append {} separately
				*/
			}

			void BasicSyntaxTranslator::HandleWhile(AST::ASTNode& node, PseudoTranslationState& pts, recursive_handler rhandler)
			{

				CompilationInfo::LabelID check_label = pts.cs_observer->GetGST().GetNewLabelID();

				CompilationInfo::LabelID end_label = pts.cs_observer->GetGST().GetNewLabelID();
			

				/*
				while (condition)
				{
				
				}
				
				while (condition) statement
				
				*/
				if (node.tokens.size() < 4)
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Cycle's block[while] is invalid. Cycle's header structure is invalid.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
					return;
				}
				if (node.tokens[0].type != TokenType::KEYWORD || node.tokens[0].value.strVal != Keywords::w_while)
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Cycle's block[while] is invalid. Cycle's identifier is invalid.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
					return;
				}
				//condition building

				int parentheses_depth = 0;

				AST::ASTNode condition;
				condition.type = AST::ASTNodeType::Expression;
				AST::ASTNode line_statement;
				line_statement.type = AST::ASTNodeType::Expression;
				size_t ci = 1; //skip while
				bool build_condition = true;
				for (; ci < node.tokens.size(); ci++)		// while ([1]
				{
					const Token& t = node.tokens[ci];
					if (t.type == TokenType::DELIMITER && t.value.strVal == "(" && build_condition)
					{
						if (parentheses_depth == 0)
						{
							parentheses_depth++; continue;
						}
						parentheses_depth++;
					}
					if (t.type == TokenType::DELIMITER && t.value.strVal == ")" && build_condition)
					{
						parentheses_depth--;
						if (parentheses_depth == 0)
						{
							build_condition = false;
							continue;
						}
					}
					if (build_condition) condition.tokens.push_back(t);
					else
					{
						line_statement.tokens.push_back(t);
					}
				}

				if (condition.tokens.empty())
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Cycle's block[while] is invalid. Condition's expression is null.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.declaring_place));
					return;
				}
				
				//Add label
				PseudoOperation conditionlabel = PseudoOperation(PseudoOpCode::Label, check_label, node.declaring_place);
				pts.pseudo_code.Pushback(conditionlabel);
				rhandler(condition, pts);

				PseudoOperation jump_label = PseudoOperation(PseudoOpCode::JumpNIf, end_label, node.declaring_place);
				pts.pseudo_code.Pushback(jump_label);

				size_t current_index = pts.pseudo_code.Size();
				if (!line_statement.tokens.empty())
				{
					rhandler(line_statement, pts);	// line_statement handling
				}
				else
				{
					for (auto body_line : node.children) rhandler(body_line, pts);	//WARNING! keep controlling visible scopes
				}

				//It will be Checking on break, continue keywords

				for (;current_index < pts.pseudo_code.Size(); current_index++)
				{
					PseudoOperation& po = pts.pseudo_code[current_index];
					if (!(po.flags & PseudoOperationFlags::Unhandled)) continue;
					if (po.flags & PseudoOperationFlags::Break)
					{
						po.arg_0 = end_label;
					}
					else if (po.flags & PseudoOperationFlags::Continue)
					{
						po.arg_0 = check_label;
					}
					else 
					{
						continue;
					}

					po.flags &= ~(PseudoOperationFlags::Unhandled | PseudoOperationFlags::Break | PseudoOperationFlags::Continue);

				}
				//jump to heck
				jump_label = PseudoOperation(PseudoOpCode::Jump, check_label, node.declaring_place);
				pts.pseudo_code.Pushback(jump_label);
				//end label
				PseudoOperation endlabel = PseudoOperation(PseudoOpCode::Label, end_label, node.declaring_place);
				pts.pseudo_code.Pushback(endlabel);
				if (!line_statement.tokens.empty()) //Insert code_block after line_statement and jump (if this block is last)
				{
					for (auto body_line : node.children) rhandler(body_line, pts);//WARNING! keep controlling visible scopes
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
					case AST::ASTNodeType::WhileCycle: 
						HandleWhile(node, pts, rhandler);
						break;
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
