#include "..\..\..\..\include\IR\PseudoTranslation\Postfix\APSTBuilder.hpp"

namespace MSLC
{
	using namespace Tokenization;
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{

			TokensGroup APSTBuilder::HandleOperator(std::vector<TokensGroup>& stack, TokensGroup& _operator)
			{
				if (!OperatorsTable::Get().Has(_operator.simple.value.strVal))
				{
					//InsertError(maybe with type Development)
					std::cout << "Error 3 (APSTBuilder)\n";
					return{};
				}
				OperatorInfo info = OperatorsTable::Get().GetInfo(_operator.simple.value.strVal);

				if (info.type == OperatorInfo::Type::Unary)
				{
					if (stack.empty())
					{
						//InsertError
						std::cout << "Error 4 (APSTBuilder)\n";
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
						std::cout << "Error 5 (APSTBuilder)\n";
						return{};
					}
					auto right = stack.back(); stack.pop_back();
					auto left = stack.back(); stack.pop_back();
					std::vector<TokensGroup> group_children = { left,right };
					TokensGroup new_group(_operator, group_children, GroupType::Operation);
					return new_group;
				}
				if (info.type == OperatorInfo::Type::Assignment) //Maybe declaration they 
				{
					if (stack.size() < 2)
					{
						//InsertError
						std::cout << "Error 6 (APSTBuilder)\n";
						return {};
					}
					auto right = stack.back(); stack.pop_back();	//saved value
					if (stack.size() > 1) //TODO Check on arrays!
					{
						std::vector<TokensGroup> declaration_args = stack;
						stack.clear();
						TokensGroup new_group(declaration_args, GroupType::Declaration);
						std::vector<TokensGroup> gp = { new_group,right };
						TokensGroup assignment(_operator, gp, GroupType::Operation);
						return assignment;
					}
					else 
					{
						auto left = stack.back(); stack.pop_back();
						std::vector<TokensGroup> gp = { left,right };
						TokensGroup assignment(_operator, gp, GroupType::Operation);
						return assignment;
					}
				}
				std::cout << "Error 7 (APSTBuilder::HandleOperator)\n";
				return {};
			}
			TokensGroup APSTBuilder::HandleToOperator(std::vector<TokensGroup>& stack, TokensGroup& to_op)
			{
				// Needed: [expression] to [type]
				if (stack.size() < 2)
				{
					std::cout << "Error 8 (APSTBuilder::HandleToOperator)\n";
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
					std::cout << "Error 9 (APSTBuilder::HandleNewOperator)\n";
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
							else {
								std::stack<TokensGroup> modifiers_stack = {};
								
								while (!stack.empty()) 
								{
									auto gp = stack.back();
									if (gp.IsSimple() && gp.simple.type == TokenType::UNDEFINED) 
									{
										stack.pop_back();
										continue;
									}
									if (gp.IsSimple() && gp.simple.type == TokenType::TYPE_MARKER)
									{		
										modifiers_stack.push(gp);
										stack.pop_back();
									}
									else break;

								}
								if (modifiers_stack.empty())
								{
									stack.push_back(group);
									continue;
								}
								std::vector<TokensGroup> modifiers = {};
								while (!modifiers_stack.empty())
								{
									modifiers.push_back(modifiers_stack.top());
									modifiers_stack.pop();
								}
								TokensGroup new_group(group, modifiers, GroupType::Type);
								stack.push_back(new_group);
								break;
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
							if (stack.empty()) 
							{
								std::cout << "Error 10 (APSTBuilder::Complex::DataAccess)\n";
								continue;
							}
							if (stack.back().type == GroupType::DataAccessChain) 
							{
								stack.back().complex.push_back(group);
							}
							else 
							{
								std::vector<TokensGroup> args = { stack.back(),group };
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
					TokensGroup(stack, GroupType::Root);
					return TokensGroup(stack, GroupType::Root);
				}

				return TokensGroup();
			}
		}
	}
}