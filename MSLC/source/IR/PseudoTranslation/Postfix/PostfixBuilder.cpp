#include "..\..\..\..\include\IR\PseudoTranslation\Postfix\PostfixBuilder.hpp"

namespace MSLC
{
    using namespace Tokenization;
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
            PostfixBuilder::PostfixBuilder()
            {
             
            }


            GroupType PostfixBuilder::IdentificateGroupType(const std::vector<Tokenization::Token>& tokens, size_t current_index)
            {
                const Token& t = tokens[current_index];

                if (t.type == TokenType::IDENTIFIER && current_index + 1 < tokens.size() && 
                    tokens[current_index + 1].type == TokenType::DELIMITER && 
                    tokens[current_index + 1].value.strVal == "(") return GroupType::FunctionCall;


                if (tokens[current_index].type == TokenType::DELIMITER && tokens[current_index].value.strVal == "[")
                {
                    return GroupType::DataAccess;
                }


                if (t.type == TokenType::ATTRIBUTE) return GroupType::AttributeUsing;
                return GroupType::Simple;
            }

            TokensGroup PostfixBuilder::HandleFuncCall(const std::vector<Tokenization::Token>& tokens, size_t& current_index)
            {
                // Found func call - create complex group
                Token t = tokens[current_index];
                TokensGroup func_call(GroupType::FunctionCall);
                func_call.simple.line = t.line;	//for debugging
                func_call.simple = t; // function identifier

                current_index += 2; // skip name and (
                std::vector<Token> args_tokens;
                int depth = 1;

                // Collect argument with recursion
                while (current_index < tokens.size() && depth > 0) {
                    if (tokens[current_index].value.strVal == "(") depth++;
                    else if (tokens[current_index].value.strVal == ")") depth--;
                    else if (tokens[current_index].value.strVal == "," && depth == 1) {
                        // Founded argument's separator
                        if (!args_tokens.empty()) {
                            TokensGroup arg(GroupType::Argument);
                            auto arg_postfix = BuildPostfix(args_tokens);

                            if (arg_postfix.IsSimple()) 
                            {
                                arg.complex.push_back(arg_postfix);
                            }
                            else if (arg_postfix.type == GroupType::Root)
                            {
                                arg.complex.insert(arg.complex.end(),
                                    arg_postfix.complex.begin(), arg_postfix.complex.end());
                                args_tokens.clear();  
                            }
                            else 
                            {
                                arg.complex.push_back(arg_postfix);
                            }
                            func_call.complex.push_back(arg);
                        }
                        current_index++;
                        continue;
                    }

                    if (depth > 0) {
                        args_tokens.push_back(tokens[current_index]);
                    }
                    current_index++;
                }
                current_index--; // compinsation
                if (!args_tokens.empty()) {
                    TokensGroup arg(GroupType::Argument);
                    auto arg_postfix = BuildPostfix(args_tokens);

                    if (arg_postfix.IsSimple())
                    {
                        arg.complex.push_back(arg_postfix);
                    }
                    else
                    {
                        arg.complex.insert(arg.complex.end(),
                            arg_postfix.complex.begin(), arg_postfix.complex.end());
                        args_tokens.clear();

                    }
                    func_call.complex.push_back(arg);
                }
                //Logger::Get().PrintInfo("ToPostfix: discovered function's call \"" + func_call.tokens[0].token.value.strVal + "\" with " + std::to_string(func_call.tokens.size() - 2) + " arguments.");
                return func_call;
            }

            TokensGroup PostfixBuilder::HandleDataAccess(const std::vector<Tokenization::Token>& tokens, size_t& current_index)
            {
                // Found func call - create complex group
                Token t = tokens[current_index];
                TokensGroup data_access(GroupType::DataAccess);

                if (t.type == TokenType::DELIMITER)
                {
                    current_index += 1; // skip [
                }
                std::vector<Token> args_tokens;
                int depth = 1;

                // Collect argument with recursion
                while (current_index < tokens.size() && depth > 0) {
                    if (tokens[current_index].value.strVal == "[") depth++;
                    else if (tokens[current_index].value.strVal == "]") depth--;
                    if (tokens[current_index].value.strVal == "]" && depth == 0) {
                        if (!args_tokens.empty()) {
                            TokensGroup arg(GroupType::Argument);
                            auto arg_postfix = BuildPostfix(args_tokens);

                            if (arg_postfix.IsSimple())
                            {
                                arg.complex.push_back(arg_postfix);
                            }
                            else if (arg_postfix.type == GroupType::Root)
                            {
                                arg.complex.insert(arg.complex.end(),
                                    arg_postfix.complex.begin(), arg_postfix.complex.end());
                                args_tokens.clear();
                            }
                            else
                            {
                                arg.complex.push_back(arg_postfix);
                            }
                            data_access.complex.push_back(arg);
                        }
                        break;
                    }

                    if (depth > 0) {
                        args_tokens.push_back(tokens[current_index]);
                    }
                    current_index++;
                }
                //Logger::Get().PrintInfo("ToPostfix: discovered function's call \"" + func_call.tokens[0].token.value.strVal + "\" with " + std::to_string(func_call.tokens.size() - 2) + " arguments.");
                return data_access;
            }
            
            TokensGroup PostfixBuilder::BuildPostfix(const std::vector<Tokenization::Token>& tokens)
            {
                if (tokens.size() == 1) return TokensGroup(tokens.front());
                std::vector<TokensGroup> operations;		// i dont want use the stack, because it doesnt have index acception
                std::vector<TokensGroup> result;		//identifiers and literals
                for (size_t i = 0; i < tokens.size(); i++)
                {
                    GroupType gtype = IdentificateGroupType(tokens, i);
                
                    switch (gtype)
                    {
                    case MSLC::IntermediateRepresentation::Pseudo::GroupType::Simple:
                    {
                        const Token& t = tokens[i];
                        //Main algorythm
                        if (t.type == TokenType::IDENTIFIER || t.type == TokenType::LITERAL || (t.type == TokenType::KEYWORD && !OperatorsTable::Get().Has(t.value.strVal)) || t.type == TokenType::TYPE_MARKER) {
                            result.push_back(TokensGroup(t));
                        }
                        else if ((t.type == TokenType::OPERATOR || t.type == TokenType::KEYWORD) && OperatorsTable::Get().Has(t.value.strVal)) {
                            try 
                            {
                                while (!operations.empty() && 
                                    operations.back().simple.value.strVal != "(" &&  // dont push out"("
                                    (OperatorsTable::Get().GetInfo(operations.back().simple.value.strVal).priority >= OperatorsTable::Get().GetInfo(t.value.strVal).priority)) {
                                    result.push_back(operations.back());
                                    operations.pop_back();
                                }
                                operations.push_back(TokensGroup(t));
                            }
                            catch (...) 
                            {
                                for (auto tg : operations) 
                                {
                                    std::cout << tg.IsSimple() << " " << tg.simple.value.ToString() << "\n";
                                }
                                int i = 0;
                            }
                        }
                        else if (t.value.strVal == "(") {  // ← Simple delimiter handling
                            operations.push_back(TokensGroup(t));
                        }
                        else if (t.value.strVal == ")") {
                            while (!operations.empty() && operations.back().simple.value.strVal != "(") {
                                result.push_back(operations.back());
                                operations.pop_back();
                            }
                            if (!operations.empty()) operations.pop_back(); // deleting "("
                        }
                    }    
                        break;
                    case MSLC::IntermediateRepresentation::Pseudo::GroupType::FunctionCall:
                        result.push_back(HandleFuncCall(tokens, i));
                        break;
                    case MSLC::IntermediateRepresentation::Pseudo::GroupType::DataAccess:
                        result.push_back(HandleDataAccess(tokens, i));
                        break;
                    case MSLC::IntermediateRepresentation::Pseudo::GroupType::AttributeUsing:
                        break;
                        break;
                    default:
                        break;
                    }
                }
                while (!operations.empty()) {
                    result.push_back(operations.back());
                    operations.pop_back();
                }

                return TokensGroup(result);
			}
		}
	}
}