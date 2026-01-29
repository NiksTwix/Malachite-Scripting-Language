#include "..\..\..\..\include\IR\PseudoTranslation\Postfix\APSTBuilder.hpp"

namespace MSLC
{
	using namespace Tokenization;
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			namespace Dia = Diagnostics;
			TokensGroup APSTBuilder::HandleOperator(std::vector<TokensGroup>& stack, TokensGroup& _operator)
			{
				if (!OperatorsTable::Get().Has(_operator.simple.value.strVal))
				{
					Dia::Logger::Get().Print(Dia::InformationMessage("APSTBuilder: Undefined operator.", Dia::MessageType::DeveloperError, Dia::SourceCode, _operator.line));
					return {};
				}
				OperatorInfo info = OperatorsTable::Get().GetInfo(_operator.simple.value.strVal);

				if (info.type == OperatorInfo::Type::Unary)
				{
					if (stack.empty())
					{
						Dia::Logger::Get().Print(Dia::InformationMessage("Unary operator expects operand.", Dia::MessageType::SyntaxError, Dia::SourceCode, _operator.line));
						return{};
					}
					auto left = stack.back(); stack.pop_back();
					std::vector<TokensGroup> group_children = { left };
					TokensGroup new_group(_operator, group_children, GroupType::Operation);
					return new_group;
				}

				if (info.type == OperatorInfo::Type::Binary)
				{
					if (stack.size() < 2)
					{
						//InsertError
						Dia::Logger::Get().Print(Dia::InformationMessage("Binary operator expects two operands.", Dia::MessageType::SyntaxError, Dia::SourceCode, _operator.line));
						return{};
					}
					auto right = stack.back(); stack.pop_back();
					auto left = stack.back(); stack.pop_back();
					std::vector<TokensGroup> group_children = { left,right };
					TokensGroup new_group(_operator, group_children, GroupType::Operation);
					return new_group;
				}
				if (info.type == OperatorInfo::Type::Declaration) 
				{
					if (stack.size() < 2)
					{
						//InsertError
						Dia::Logger::Get().Print(Dia::InformationMessage("Declaration's operator expects left and right values.", Dia::MessageType::SyntaxError, Dia::SourceCode, _operator.line));
						return {};
					}
					auto right = stack.back(); stack.pop_back();	//type value
					auto left = stack.back(); stack.pop_back();		//identifier value
					std::vector<TokensGroup> declaration_args = { left,right };
					TokensGroup new_group(declaration_args, GroupType::Declaration);
					return new_group;
				}
				if (info.type == OperatorInfo::Type::Assignment) //Maybe declaration they 
				{
					if (stack.size() < 2)
					{
						//InsertError
						Dia::Logger::Get().Print(Dia::InformationMessage("Assignment's operator expects left and right values.", Dia::MessageType::SyntaxError, Dia::SourceCode, _operator.line));
						return {};
					}
					auto right = stack.back(); stack.pop_back();

					auto left = stack.back(); stack.pop_back();
					std::vector<TokensGroup> gp = { left,right };
					TokensGroup assignment(_operator, gp, GroupType::Operation);
					return assignment;

				}
				Dia::Logger::Get().Print(Dia::InformationMessage("APSTBuilder: Operator hasnt be handled.", Dia::MessageType::DeveloperError, Dia::SourceCode, _operator.line));
				return {};
			}
			TokensGroup APSTBuilder::HandleToOperator(std::vector<TokensGroup>& stack, TokensGroup& to_op)
			{
				// Needed: [expression] to [type]
				if (stack.size() < 2)
				{
					Dia::Logger::Get().Print(Dia::InformationMessage("Type casting operator expects two operands.", Dia::MessageType::SyntaxError, Dia::SourceCode, to_op.line));
					return TokensGroup();
				}
				TokensGroup target_type = stack.back(); stack.pop_back();
				TokensGroup expr = stack.back(); stack.pop_back();

				// Just grouping without semantic
				std::vector<TokensGroup> children = { expr, to_op, target_type };
				return TokensGroup(children, GroupType::TypeCast);
			}
			TokensGroup APSTBuilder::HandleNewOperator(std::vector<TokensGroup>& stack, TokensGroup& new_op)
			{
				// Нужно: new тип [аргументы/размер]
				if (stack.empty()) {
					Dia::Logger::Get().Print(Dia::InformationMessage("Object's creation operator expects one operand.", Dia::MessageType::SyntaxError, Dia::SourceCode, new_op.line));
					return TokensGroup();
				}

				TokensGroup type = stack.back(); stack.pop_back();

				// Базовая группировка
				std::vector<TokensGroup> children = { new_op, type };
				return TokensGroup(children, GroupType::NewExpression);
			}
			TokensGroup APSTBuilder::BuildAPST(TokensGroup& root)
			{
				if (root.IsSimple()) return root;
				std::vector<TokensGroup> stack;
				for (size_t i = 0; i < root.complex.size(); i++)
				{
					TokensGroup& group = root.complex[i];
					if (group.IsSimple()) 
					{
						switch (group.simple.type)
						{
						case TokenType::IDENTIFIER:
						{
							if (stack.empty())
							{
								stack.push_back(group);
								continue;
							}
						}
							
							
							
						case TokenType::LITERAL:
							stack.push_back(group);
							break;
						case TokenType::TYPE_MARKER:	
							stack.push_back(group);
							break;
						case TokenType::OPERATOR:
						{
							stack.push_back(HandleOperator(stack, group));
							break;
						}
						case TokenType::KEYWORD:
							if (!OperatorsTable::Get().Has(group.simple.value.strVal))
							{
								stack.push_back(group);
							}
							else 
							{
								auto info = OperatorsTable::Get().GetInfo(group.simple.value.strVal);
								if (info.type == OperatorInfo::Type::Meta)
								{
									if (group.simple.value.strVal == Keywords::w_to) 
									{
										stack.push_back(HandleToOperator(stack, group));
									}
									else if (group.simple.value.strVal == Keywords::w_new)
									{
										stack.push_back(HandleNewOperator(stack, group));
									}
									continue;
									//TODO Special handlers 
								}
							}
							break;
						default:
							break;
						}
					}
					else 
					{
						group.complex = BuildAPST(group).complex;
						if (group.type == GroupType::DataAccess)
						{
							if (group.complex.size() == 0)
							{
								Dia::Logger::Get().Print(Dia::InformationMessage("Operator [] expects one argument.", Dia::MessageType::SyntaxError, Dia::SourceCode, group.line));
								continue;
							}

							else if (auto& back = stack.back(); back.type == GroupType::DataAccessChain)
							{
								back.complex.push_back(group);
							}
							
							else //Creates new chain
							{
								std::vector<TokensGroup> args = { back,group };
								stack.pop_back();
								TokensGroup group1 = TokensGroup(args, GroupType::DataAccessChain);
								stack.push_back(group1);
							}
						}
						else stack.push_back(group);
					}
				}

				if (!stack.empty()) 
				{
					return TokensGroup(stack, GroupType::Root);
				}
				//Dia::Logger::Get().Print(Dia::InformationMessage("APSTBuilder: stack is empty", Dia::MessageType::DeveloperError, Dia::SourceCode, 0));
				return TokensGroup();
			}
		}
	}
}