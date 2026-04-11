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
					HandleSimple(node, pts);
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
					for (auto& child : node.complex) 
					{
						AnalyzeAPST(child,pts);
					}
					break;
				case GroupType::FunctionCall:
					break; 
				case GroupType::DataAccess:
				{
					for (auto& child : node.complex)
					{
						AnalyzeAPST(child, pts);
					}
					pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::GetByArgument, 0, 0, 0, node.debug_info, PseudoOperationFlags::None));
				}
					break;
				case GroupType::DataAccessChain:
				case GroupType::Argument:
					for (auto& child : node.complex)
					{
						AnalyzeAPST(child, pts);
					}
					break;

				case GroupType::AttributeUsing:
					break;
				
				case GroupType::Operation:
				{
					auto operator_info = OperatorsTable::Get().GetInfo(node.simple.value.strVal);
					
					if (operator_info.type == OperatorInfo::Type::Unary)
					{
						AnalyzeAPST(node.complex.front(),pts);
						pts.pseudo_code.Pushback(PseudoOperation(operator_info.op_code,0,0,0, node.debug_info,PseudoOperationFlags::Unary));
					}
					else if (operator_info.type == OperatorInfo::Type::Binary)
					{
						AnalyzeAPST(node.complex.front(), pts);	//left
						AnalyzeAPST(node.complex.back(), pts);	//right
						pts.pseudo_code.Pushback(PseudoOperation(operator_info.op_code, 0, 0, 0, node.debug_info, PseudoOperationFlags::Binary));
					}
					else if (operator_info.type == OperatorInfo::Type::Assignment) 
					{
						PseudoTranslationState temp_pts;
						temp_pts.cs_observer = pts.cs_observer;

						AnalyzeAPST(node.complex.back(), temp_pts);	//right | First

						AnalyzeAPST(node.complex.front(), pts);	//left
						if (pts.pseudo_code.Back().op_code == PseudoOpCode::DeclareVariable)
						{
							pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::Use, pts.pseudo_code.Back().arg_0, 0, 0,node.debug_info));
						}

						pts.pseudo_code.Pushback(temp_pts.pseudo_code);	//Merging

						if (pts.pseudo_code.Back().op_code == PseudoOpCode::Assign) pts.pseudo_code.Back().op_code = PseudoOpCode::AssignR;	//Check info in note in telegram group
						pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::Assign, (size_t)operator_info.op_code, 0, 0, node.debug_info, PseudoOperationFlags::Binary));
					}
				}
					break;
				case GroupType::Declaration://[variable]:[modifiers][type's identifier]
				{
					auto& identifier = node.complex.front();
					auto& type = node.complex.back();
					auto type_info = GetValueInfoFromType(type, pts);
					auto symbol = pts.cs_observer->FindSymbolLocal(identifier.simple.value.strVal, false);	//finds in only current scope
					if (symbol != nullptr) 
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Variable \"" + identifier.simple.value.strVal +  "\" has already declared.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, identifier.debug_info));
						return;
					}
					CompilationInfo::Variables::VariableDescription _desc;
					_desc.name = identifier.simple.value.strVal;
					_desc.vinfo = type_info;
					
					symbol = pts.cs_observer->RegisterVariable(_desc);
					pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::DeclareVariable, symbol->description_id, node.debug_info));
				}
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
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined identifier \"" + node.simple.value.ToString() + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
							return;
						}
						pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::Use, symbol->description_id,0,0, node.debug_info));
					}
					else if (node.simple.type == TokenType::LITERAL) 
					{
						auto constant = pts.cs_observer->GetICT().GetOrAdd(node.simple.value);
						pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::UseConstant, constant, 0, 0, node.debug_info));
					}
					else 
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined simple tokens group \"" + node.simple.value.ToString() + "\".", Diagnostics::DeveloperError, Diagnostics::SourceCode, node.debug_info));
					}
					break;
				case GroupType::ArrayLiteral:
					for (auto arg : node.complex)
					{
						AnalyzeAPST(arg, pts);	
					}
					pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::CreateArray, node.complex.size(), 0, 0, node.debug_info));
					break;
				case GroupType::QualifiedName:
				{
					CompilationInfo::Symbol* current = nullptr;
					for (size_t i = 0; i < node.complex.size(); i++) 
					{
						auto& identifier = node.complex[i];
						
						if (identifier.simple.type != TokenType::IDENTIFIER) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid identifier \"" + identifier.simple.value.ToString() + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
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
									Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined identifier \"" + str_val + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
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
								pts.pseudo_code.Pushback(PseudoOperation(PseudoOpCode::Use, current->description_id, 0, 0, node.debug_info));
								return;
							}
						}
						if (current == nullptr) 
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined identifier \"" + str_val + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
							return;
						}
					}
				}
					break;
				}
			}

			CompilationInfo::Values::ValueInfo  ExpressionsTranslator::GetValueInfoFromType(TokensGroup& node, PseudoTranslationState& pts)
			{
				
				using namespace CompilationInfo::Values;

				ValueInfo info = ValueInfo();
				if (node.type == GroupType::Simple || node.type == GroupType::QualifiedName)
				{ 
					size_t before = pts.pseudo_code.Size();
					HandleSimple(node, pts);
					bool added_one_use = (pts.pseudo_code.Size() - before == 1) && (pts.pseudo_code.Back().op_code == PseudoOpCode::Use);
					if (added_one_use)
					{
						if (!pts.cs_observer->GetGST().HasType(pts.pseudo_code.Back().arg_0))
						{
							Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined type identifier \"" + node.simple.value.ToString() + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
							return {};
						}
						info.type_id = pts.pseudo_code.Back().arg_0;	//type_description
						pts.pseudo_code.Popback();
					}
					else
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid type identifier.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
						return {};
					}
					return info;
				}

				//Type identifier with modificators

				bool was_const = false;
				for (size_t i = 0; i < node.complex.size() - 1; i++) // checks identificators
				{
					auto child = node.complex[i];

					if (child.simple.type == TokenType::TYPE_MARKER)
					{
						auto str_val = child.simple.value.strVal;
						if (str_val == TypeMarkers::w_ptr) 
						{
							info.flags = info.flags | ValueFlags::Pointer;
							if (was_const) { info.flags = info.flags | ValueFlags::ConstPointer; was_const = false; }
							info.pointers_depth += 1;
						}
						if (str_val == TypeMarkers::w_ref) { 
							info.flags = info.flags | ValueFlags::Reference;
							if (was_const) { info.flags = info.flags | ValueFlags::ConstValue; was_const = false; }
						}
						if (str_val == TypeMarkers::w_const)was_const = true;
						//if (str_val == TypeMarkers::w_gc)flags = flags | VF::GC;
						//{
						//	//Pointer + gc
						//}
					}
					else 
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Type's identifier must be one and after modifiers.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
						return {};
					}
				}
				if (was_const) info.flags = info.flags | ValueFlags::ConstValue;

				auto& identifier = node.complex.back();

				if (identifier.simple.type != TokenType::IDENTIFIER && identifier.type != GroupType::QualifiedName) 
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid type identifier.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
					return {};
				}
				size_t first = pts.pseudo_code.Size();
				HandleSimple(identifier,pts);
				bool added_one_use = (pts.pseudo_code.Size() - first == 1) && (pts.pseudo_code.Back().op_code == PseudoOpCode::Use);
				if (added_one_use)
				{
					if (!pts.cs_observer->GetGST().HasType(pts.pseudo_code.Back().arg_0)) 
					{
						Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Undefined type identifier \"" + identifier.simple.value.ToString() + "\".", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
						return {};
					}
					info.type_id = pts.pseudo_code.Back().arg_0;	//type_description
					pts.pseudo_code.Popback();
				}
				else Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid type identifier.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
				
				if ((info.flags & ValueFlags::Pointer) && (info.flags & ValueFlags::Reference))
				{
					Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("References cannot be combined with pointer modifier.", Diagnostics::SyntaxError, Diagnostics::SourceCode, node.debug_info));
					return {};
				}
				
				return info;
			}

			void ExpressionsTranslator::AnalyzeExpression(std::vector<Tokenization::Token> tokens, PseudoTranslationState& pts)
			{
				auto p = postfix_builder.Build(tokens);
				int depth = 0;
				//print_recursive(p, depth);
				auto p1 = apst_builder.BuildAPST(p);
				
				
				AnalyzeAPST(p1, pts);


				
			}
		}
	}
} 