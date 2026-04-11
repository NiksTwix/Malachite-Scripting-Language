#include "..\..\include\Preprocessing\Preprocessor.hpp"


namespace MSLC
{
	namespace Preprocessing
	{
        void Preprocessor::HandleDefineConstant(PreprocessingState& state)
        {
            // Пропускаем саму директиву
            state.tokens->at(state.current_index).type = TokenType::UNDEFINED;
            state.current_index++;
            int size = state.tokens->size();

            std::vector<Token> args;

            while (state.current_index < size)
            {
                Token& t = state.tokens->at(state.current_index);
                if (t.type == TokenType::COMPILATION_LABEL && t.value.uintVal == (uint64_t)CompilationLabel::OPERATION_END) break;	//Handles before end of operation (in our case its end of line)
                args.push_back(t);
                t.type = TokenType::UNDEFINED;

                state.current_index++;
            }
            if (args.size() < 3)
            {
                Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid arguments count of directive \"" + std::string(Directives::w_define_const) + "\".Expected NAME = VALUE/EXPRESSION.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.current_line));
                return;
            }
            else if (args.size() == 3)   //Simple expression
            {
                Token& name_token = args.at(0);
                Token& equals_token = args.at(1);
                Token& value_token = args.at(2);

                // Проверяем структуру

                if (name_token.type != TokenType::IDENTIFIER ||equals_token.value.strVal != "=")
                {
                    Diagnostics::Logger::Get().Print({ "Invalid \"" + std::string(Directives::w_define_const) + "\" directive's identifier.",  Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, state.current_line });
                    return;
                }
                if (equals_token.value.strVal != "=")
                {
                    Diagnostics::Logger::Get().Print({ "Directive \"" + std::string(Directives::w_define_const) + "\" excepts = after identifier, but gets \""+ equals_token.value.strVal +"\".",  Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, state.current_line });
                    return;
                }
                // Macro's registration
                std::string name = name_token.value.strVal;
                if (value_token.type == TokenType::LITERAL)state.c_state->GetMacrosTable().DefineConstant(name, value_token.value, state.current_line);
                else if (value_token.type == TokenType::IDENTIFIER && state.c_state->GetMacrosTable().HasMacro(value_token.value.strVal))
                {
                    auto macro = state.c_state->GetMacrosTable().GetMacro(value_token.value.strVal);

                    if (macro.type != MacroType::Constant) 
                    {
                        Diagnostics::Logger::Get().Print({ "Directive \"" + std::string(Directives::w_define_const) + "\" supports only constant macros in its arguments.",  Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, state.current_line });
                        return;
                    }
                    state.c_state->GetMacrosTable().DefineConstant(name, macro.constant_value, state.current_line);
                }
                else 
                {
                    Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid arguments of directive \"" + std::string(Directives::w_define_const) + "\".", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.current_line));
                    return;
                }
            }
            else 
            {
                //TODO: Complex Expressions: PseudoTranslator->ConstantFolder->PseudoOperation.value
            }        
        }

        void Preprocessor::HandleDefineInsertion(PreprocessingState& state)
        {
            state.tokens->at(state.current_index).type = TokenType::UNDEFINED;
            state.current_index++;  //Skip directive
            int depth = 0;   //For {}
            Token identifier = state.tokens->at(state.current_index);
            state.tokens->at(state.current_index).type = TokenType::UNDEFINED;
            state.current_index++;

            size_t end = state.tokens->size();

            std::vector<ParameterLabel> header;
            std::vector<Token> body;

            uint8_t arg_name_tokens_count = 0;

            bool header_f = false;
            bool body_f = false;
            for (; state.current_index < end; state.current_index++) 
            {
                Token t = state.tokens->at(state.current_index);
                state.tokens->at(state.current_index).type = TokenType::UNDEFINED;
                if (t.type == TokenType::DELIMITER && t.value.strVal == "(" && !header_f)
                {
                    header_f = true;
                    continue;
                }
                if (t.type == TokenType::DELIMITER && t.value.strVal == ")" && header_f)
                {
                    if (arg_name_tokens_count > 2)
                    {
                        Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid argument is in macro's \"" + identifier.value.strVal + "\" header.", Diagnostics::SyntaxError, Diagnostics::SourceCode, identifier.debug_info));
                    }
                    header_f = false;
                    continue;
                }
                if (t.type == TokenType::DELIMITER && t.value.strVal == "," && header_f)
                {
                    if (arg_name_tokens_count != 1) 
                    {
                        Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid argument is in macro's \"" + identifier.value.strVal + "\" header.", Diagnostics::SyntaxError, Diagnostics::SourceCode, identifier.debug_info));
                    }
                    arg_name_tokens_count = 0;
                    continue;
                }
                if (header_f)
                {
                    ParameterLabel label;
                    label.name = t.value.strVal;
                    label.id = header.size();
                    header.push_back(label);
                    arg_name_tokens_count += 1;
                }
                else 
                {
                    if (t.type == TokenType::DELIMITER && t.value.strVal == "{")
                    {
                        depth++;
                        if (depth - 1 == 0)
                        {
                            body_f = true;
                            continue;
                        }
                       
                    }
                    if (t.type == TokenType::DELIMITER && t.value.strVal == "}" && body_f)
                    {
                        depth--;
                        if (depth == 0) break;
                    }

                    if (t.type == TokenType::IDENTIFIER) 
                    {
                        for (auto t1 : header) 
                        {
                            if (t1.name == t.value.strVal) 
                            {
                                t.type = TokenType::PREPROCESSOR_DIRECTIVE_ARG;
                                t.value.uintVal = t1.id;
                            }
                        }
                    }
                    body.push_back(t);
                }

                
            }
            state.c_state->GetMacrosTable().DefineFunction(identifier.value.strVal, body, header, identifier.debug_info);

        }

        void Preprocessor::Postprocess(CompilationInfo::CompilationState& c_state, std::vector<Token>& tokens)
        {
            std::vector<Token> source = tokens;
            tokens.clear();

            for (size_t i = 0; i < source.size(); i++) 
            {
                Token& t = source[i];

                if (t.type == TokenType::UNDEFINED) continue;

                if (t.type == TokenType::PREPROCESSOR_DIRECTIVE_CALL) 
                {
                    auto* macro = c_state.GetMacrosTable().TryGetMacro(t.value.strVal);

                    if (macro == nullptr)
                    {
                        Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Attempt of undefined macro \"" + t.value.strVal + "\" calling exists.", Diagnostics::SyntaxError, Diagnostics::SourceCode, t.debug_info));
                        return;
                    }


                    if (macro->type == MacroType::Constant) {
                        t.type = TokenType::LITERAL;
                        t.value = macro->constant_value;
                    }
                    else if (macro->type == MacroType::FunctionLike) {
                        //Skip identificator
                        i++;
                        InsertMacro(c_state, tokens, source, i, macro);
                        continue;
                    }
                }


                tokens.push_back(t);
            }

        }

        void Preprocessor::InsertMacro(CompilationInfo::CompilationState& c_state, std::vector<Token>& tokens, std::vector<Token>& source, size_t& i, MacroDefinition* macro)
        {

            std::vector<MacroArgument> args;
            MacroArgument current;
            if (source[i].type == TokenType::DELIMITER && source[i].value.strVal == "(")
            {
                int depth = 0;

                for (; i < source.size(); i++) 
                {
                    if (source[i].type == TokenType::DELIMITER && source[i].value.strVal == "(" && depth == 0)
                    {
                        continue;
                    }
                    if (source[i].type == TokenType::DELIMITER && source[i].value.strVal == "," && depth == 0)
                    {
                        if (current.tokens.empty()) Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid argument is in macros call.", Diagnostics::SyntaxError, Diagnostics::SourceCode, source[i].debug_info));
                        args.push_back(current);
                        current = MacroArgument();
                        continue;
                    }
                    if (source[i].type == TokenType::DELIMITER && source[i].value.strVal == ")" && depth == 0)
                    {
                        if (!current.tokens.empty())  args.push_back(current);

                        break;
                    }
                    if (source[i].type == TokenType::DELIMITER && source[i].value.strVal == ")") depth--;

                    if (source[i].type == TokenType::DELIMITER && source[i].value.strVal == "(") depth++;

                    
                   
                    
                    if (depth < 0) 
                    {
                        Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Extraneous ) is in macro call.", Diagnostics::SyntaxError, Diagnostics::SourceCode, source[i].debug_info));
                        return;
                    }


                    current.tokens.push_back(source[i]);
                }
            }

            if (args.size() != macro->parameters.size()) 
            {
                Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Macro's arguments are invalid.", Diagnostics::LogicError, Diagnostics::SourceCode, source[i].debug_info));
                return;
            }

            for (auto t : macro->body_tokens) 
            {
                if (t.type == TokenType::PREPROCESSOR_DIRECTIVE_ARG)
                {
                    MacroArgument& arg = args[t.value.uintVal];
                    for (auto to : arg.tokens) 
                    {
                        tokens.push_back(to);
                    }
                    continue;
                }
                tokens.push_back(t);
            }
        }


        void Preprocessor::Preprocess(CompilationInfo::CompilationState& c_state,
            std::vector<Token>& tokens)
        {
            PreprocessingState state;
            state.c_state = &c_state;
            state.tokens = &tokens;

            // Один проход, всё на месте
            for (state.current_index = 0; state.current_index < tokens.size(); state.current_index++) {
                Token& t = tokens[state.current_index];
                state.current_line = t.debug_info;  // Для сообщений об ошибках

                if (t.type == TokenType::PREPROCESSOR_DIRECTIVE) {
                    if (t.value.strVal == Directives::w_define_const) {
                        HandleDefineConstant(state);
                    }
                    if (t.value.strVal == Directives::w_define_insertion) {
                        HandleDefineInsertion(state);
                    }
                    if (t.value.strVal == Directives::w_import) {
                        if (state.current_index + 1 < tokens.size() && tokens[state.current_index + 1].type == TokenType::LITERAL && tokens[state.current_index + 1].value.type == Definitions::ValueType::STRING)
                        {
                            tokens[state.current_index + 1].type = TokenType::UNDEFINED;
                            tokens[state.current_index + 1].value.strVal = "";
                        }
                        else 
                        {
                            Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Invalid arguments count of directive \"" + std::string(Directives::w_import) + "\". Expected #import \"path to file\"", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.current_line));
                        }

                        state.current_index++;
                    }
                    // Помечаем саму директиву как UNDEFINED
                    t.type = TokenType::UNDEFINED;
                }
            }

            Postprocess(c_state, tokens);

            // Всё! Ничего не удаляем, парсер сам пропустит UNDEFINED
        }
	}

}