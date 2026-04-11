#include "..\..\..\..\include\IR\PseudoTranslation\Postfix\PostfixBuilder.hpp"
#include <unordered_set>

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

            TokensGroup PostfixBuilder::Build(const std::vector<Tokenization::Token>& tokens)
            {
                auto r1 = Preprocess(tokens);

                return BuildPostfix(r1);
            }


            GroupType PostfixBuilder::IdentificateGroupType(const std::vector<TokensGroup>& tokens, size_t current_index)
            {
                const Token& t = tokens[current_index].simple;

                static std::unordered_set<std::string> array_literal_exceptions = { "]",")","}" };

                if ((t.type == TokenType::IDENTIFIER || tokens[current_index].type == GroupType::QualifiedName) && current_index + 1 < tokens.size() &&
                    tokens[current_index + 1].simple.type == TokenType::DELIMITER && 
                    tokens[current_index + 1].simple.value.strVal == "(") return GroupType::FunctionCall;

                if (tokens[current_index].simple.type == TokenType::DELIMITER && tokens[current_index].simple.value.strVal 
                    == "[" && ((current_index > 0 && (tokens[current_index-1].simple.type == TokenType::OPERATOR 
                    || (tokens[current_index - 1].simple.type == TokenType::DELIMITER && !array_literal_exceptions.count(tokens[current_index - 1].simple.value.strVal)))) 
                    || current_index == 0) )
                {
                    return GroupType::ArrayLiteral;
                }
                if (tokens[current_index].simple.type == TokenType::DELIMITER && tokens[current_index].simple.value.strVal == "[")
                {
                    return GroupType::DataAccess;
                }


                if (t.type == TokenType::ATTRIBUTE) return GroupType::AttributeUsing;
                return GroupType::Simple;
            }

            TokensGroup PostfixBuilder::HandleFuncCall(const std::vector<TokensGroup>& tokens, size_t& current_index)
            {
                // Found func call - create complex group
                TokensGroup t = tokens[current_index];
                TokensGroup func_call(GroupType::FunctionCall);
                func_call.line = t.line;	//for debugging
                func_call.complex.push_back(t); // function identifier

                current_index += 2; // skip name and (
                std::vector<TokensGroup> args_tokens;
                int depth = 1;

                // Collect argument with recursion
                while (current_index < tokens.size() && depth > 0) {
                    if (tokens[current_index].simple.value.strVal == "(") depth++;
                    else if (tokens[current_index].simple.value.strVal == ")") depth--;
                    else if (tokens[current_index].simple.value.strVal == "," && depth == 1) {
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
                        else
                        {
                            Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid argument.", Diagnostics::SyntaxError, Diagnostics::SourceCode, t.line));
                            current_index++;
                            break;
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

                    if (arg_postfix.IsSimpleOrQN())
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

            TokensGroup PostfixBuilder::HandleDataAccessOrArrayLiteral(const std::vector<TokensGroup>& tokens, size_t& current_index, GroupType gtype)
            {
                // Found func call - create complex group
                TokensGroup t = tokens[current_index];
                TokensGroup data_access(gtype);
                data_access.line = t.line;
                if (t.simple.type == TokenType::DELIMITER)
                {
                    current_index += 1; // skip [
                }
                std::vector<TokensGroup> args_tokens;
                int depth = 1;

                // Collect argument with recursion
                while (current_index < tokens.size() && depth > 0) {
                    if (tokens[current_index].simple.value.strVal == "[") depth++;
                    else if (tokens[current_index].simple.value.strVal == "]") depth--;
                    if ((tokens[current_index].simple.value.strVal == "," && depth == 1) || (tokens[current_index].simple.value.strVal == "]" && depth == 0)) {
                        if (!args_tokens.empty()) {
                            TokensGroup arg(GroupType::Argument);
                            auto arg_postfix = BuildPostfix(args_tokens);

                            if (arg_postfix.IsSimpleOrQN())
                            {
                                arg.complex.push_back(arg_postfix);
                            }
                            else if (arg_postfix.type == GroupType::Root)
                            {
                                arg.complex.insert(arg.complex.end(),
                                    arg_postfix.complex.begin(), arg_postfix.complex.end());
                                
                            }
                            else
                            {
                                arg.complex.push_back(arg_postfix);
                            }
                            data_access.complex.push_back(arg);
                            args_tokens.clear();
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
                //Logger::Get().PrintInfo("ToPostfix: discovered function's call \"" + func_call.tokens[0].token.value.strVal + "\" with " + std::to_string(func_call.tokens.size() - 2) + " arguments.");
                return data_access;
            }
            
            std::vector<TokensGroup> PostfixBuilder::Preprocess(const std::vector<Tokenization::Token>& tokens)
            {
                std::vector<TokensGroup> result;

                for (size_t i = 0; i < tokens.size(); i++)                
                {
                    auto token = tokens[i];
                    if (token.type == Tokenization::TokenType::OPERATOR && token.value.strVal == "::" ) 
                    {
                        if (i > 0 && result.back().simple.type == TokenType::IDENTIFIER &&  i + 1 < tokens.size() && tokens[i+1].type == Tokenization::TokenType::IDENTIFIER)
                        {
                            auto back_token = result.back();
                            result.pop_back();
                            
                            auto next_token = tokens[i + 1];

                            std::vector<TokensGroup> args = { back_token,next_token };

                            result.push_back(TokensGroup(args, GroupType::QualifiedName));
                            i++;    //skip next
                            continue;
                        }
                        else if (i > 0 && result.back().type == GroupType::QualifiedName && i + 1 < tokens.size() && tokens[i + 1].type == Tokenization::TokenType::IDENTIFIER)
                        {
                            result.back().complex.push_back(tokens[i + 1]);
                            i++;    //skip next
                            continue;
                        }
                        else 
                        {
                            Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid access to field.", Diagnostics::SyntaxError, Diagnostics::SourceCode, token.debug_info));
                            continue;
                        }
                    }
                    result.push_back(token);
                }

                return result;
            }

            TokensGroup PostfixBuilder::BuildPostfix(const std::vector<TokensGroup>& tokens)
            {
                if (tokens.size() == 1) return tokens.front();
                //if (tokens.size() == 1 && (tokens.front().IsSimple() || tokens.front().type == GroupType::QualifiedName)) return tokens.front();
                std::vector<TokensGroup> operations;		// i dont want use the stack, because it doesnt have index acception
                std::vector<TokensGroup> result;		//identifiers and literals
                for (size_t i = 0; i < tokens.size(); i++)
                {
                    GroupType gtype = IdentificateGroupType(tokens, i);
                
                    switch (gtype)
                    {
                    case MSLC::IntermediateRepresentation::Pseudo::GroupType::Simple:
                    {
                        const TokensGroup& t = tokens[i];
                        //Main algorythm

                        if (t.simple.type == TokenType::TYPE_MARKER)
                        {
                            std::vector<TokensGroup> type_modifiers;
                            type_modifiers.push_back(t);
                            i++;
                            for (size_t j = i; j < tokens.size(); j++)      //take before first identirier
                            {
                                const TokensGroup& t1 = tokens[j];
                                if (t1.simple.type == TokenType::TYPE_MARKER) type_modifiers.push_back(t1);        //ref,ptr,const and another
                                else if (t1.simple.type == TokenType::IDENTIFIER || t1.type == GroupType::QualifiedName)
                                {
                                    type_modifiers.push_back(t1);
                                    auto type_group = TokensGroup(type_modifiers, GroupType::Type);
                                    result.push_back(type_group);
                                    i = j;
                                    break;
                                }
                                else 
                                {
                                    Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid type identifier.", Diagnostics::SyntaxError, Diagnostics::SourceCode, t.line));
                                    break;
                                }
                            }
                            continue;   //  

                        }

                        if (t.simple.type == TokenType::IDENTIFIER || t.simple.type == TokenType::LITERAL || t.type == GroupType::QualifiedName) {
                            result.push_back(t);
                        }
                        else if ((t.simple.type == TokenType::OPERATOR || t.simple.type == TokenType::KEYWORD) && OperatorsTable::Get().Has(t.simple.value.strVal)) {
                            try 
                            {
                                
                                auto current_operator = t.simple.value.strVal;

                                auto get_op_info = [](const std::string& operator_) -> OperatorInfo
                                    {
                                        return  OperatorsTable::Get().GetInfo(operator_);
                                    };

                                while (!operations.empty() && operations.back().simple.value.strVal != "(") {
                                    auto back_op = operations.back().simple.value.strVal;
                                    auto back_info = get_op_info(back_op);
                                    auto current_info = get_op_info(current_operator);

                                    if (back_info.priority > current_info.priority ||
                                        (back_info.priority == current_info.priority &&
                                            current_info.associativity == OperatorInfo::Associativity::LeftRight &&
                                            !(back_info.type == OperatorInfo::Type::Unary && current_info.type == OperatorInfo::Type::Unary))) {
                                        result.push_back(operations.back());
                                        operations.pop_back();
                                    }
                                    else {
                                        break;
                                    }
                                }
                                
                                
                                operations.push_back(TokensGroup(t));
                            }
                            catch (...) 
                            {
                                for (auto tg : operations) 
                                {
                                    std::cout << tg.IsSimple() << " " << tg.simple.value.ToString() << "\n";
                                }
                            }
                        }
                        else if (t.simple.value.strVal == "(") {  // ← Simple delimiter handling
                            operations.push_back(TokensGroup(t));
                        }
                        else if (t.simple.value.strVal == ")") {
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
                    case MSLC::IntermediateRepresentation::Pseudo::GroupType::ArrayLiteral:
                    case MSLC::IntermediateRepresentation::Pseudo::GroupType::DataAccess:
                        result.push_back(HandleDataAccessOrArrayLiteral(tokens, i,gtype));
                        break;
                    case MSLC::IntermediateRepresentation::Pseudo::GroupType::AttributeUsing:
                        break;
                    default:
                        break;
                    }
                }
                while (!operations.empty()) {
                    result.push_back(operations.back());
                    operations.pop_back();
                }

                //for (auto t : result) 
                //{
                //    if (t.type == GroupType::QualifiedName) 
                //    {
                //        for (auto t2 : t.complex) 
                //        {
                //            std::cout << t2.simple.value.ToString() << " ";
                //        }
                //    }
                //
                //    else std::cout << t.simple.value.ToString() << " ";
                //}

                return TokensGroup(result);
			}
		}
	}
}