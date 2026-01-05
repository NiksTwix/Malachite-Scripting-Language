#include "..\..\include\Preprocessing\Preprocessor.hpp"


namespace MSLC
{
	namespace Preprocessing
	{
        void Preprocessor::HandleDefineConstant(PreprocessingState& state)
        {
            // Пропускаем саму директиву
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
                Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Invalid arguments count of directive \"" + std::string(Directives::w_define_const) + "\".Expected NAME = VALUE/EXPRESSION.", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.current_line));
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
                    Diagnostics::Logger::Get().PrintToCmd({ "Invalid \"" + std::string(Directives::w_define_const) + "\" directive's identifier.",  Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, state.current_line });
                    return;
                }
                if (equals_token.value.strVal != "=")
                {
                    Diagnostics::Logger::Get().PrintToCmd({ "Directive \"" + std::string(Directives::w_define_const) + "\" excepts = after identifier, but gets \""+ equals_token.value.strVal +"\".",  Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, state.current_line });
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
                        Diagnostics::Logger::Get().PrintToCmd({ "Directive \"" + std::string(Directives::w_define_const) + "\" supports only constant macros in its arguments.",  Diagnostics::MessageType::SyntaxError, Diagnostics::SourceType::SourceCode, state.current_line });
                        return;
                    }
                    state.c_state->GetMacrosTable().DefineConstant(name, macro.constant_value, state.current_line);
                }
                else 
                {
                    Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Invalid arguments of directive \"" + std::string(Directives::w_define_const) + "\".", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.current_line));
                    return;
                }
            }
            else 
            {
                //TODO: Complex Expressions: PseudoTranslator->ConstantFolder->PseudoOperation.value
            }        
        }

        void Preprocessor::HandleDirectiveCall(PreprocessingState& state)
        {
            Token& macro_token = state.tokens->at(state.current_index);
            std::string name = macro_token.value.strVal;

            auto& macro = state.c_state->GetMacrosTable().GetMacro(name);

            if (macro.type == MacroType::Constant) {
                // Просто заменяем токен на месте
                macro_token.type = TokenType::LITERAL;
                macro_token.value = macro.constant_value;
            }
            else if (macro.type == MacroType::FunctionLike) {
                macro_token.type = TokenType::PREPROCESSOR_DIRECTIVE_CALL;
            }
            // For function-like macros. Now we dont realize it. Replace main token on CompilationLabel::DirectiveCall -> handling will be in the PseudoTranslator 
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
                state.current_line = t.line;  // Для сообщений об ошибках

                if (t.type == TokenType::PREPROCESSOR_DIRECTIVE) {
                    if (t.value.strVal == Directives::w_define_const) {
                        HandleDefineConstant(state);
                    }
                    if (t.value.strVal == Directives::w_import) {
                        if (state.current_index + 1 < tokens.size() && tokens[state.current_index + 1].type == TokenType::LITERAL && tokens[state.current_index + 1].value.type == Definitions::ValueType::STRING)
                        {
                            tokens[state.current_index + 1].type = TokenType::UNDEFINED;
                        }
                        else 
                        {
                            Diagnostics::Logger::Get().PrintToCmd(Diagnostics::InformationMessage("Invalid arguments count of directive \"" + std::string(Directives::w_import) + "\". Expected #import \"path to file\"", Diagnostics::MessageType::SyntaxError, Diagnostics::SourceCode, state.current_line));
                        }

                        state.current_index++;
                    }
                    // Помечаем саму директиву как UNDEFINED
                    t.type = TokenType::UNDEFINED;
                }
                else if (t.type == TokenType::IDENTIFIER &&
                    c_state.GetMacrosTable().HasMacro(t.value.strVal)) {
                    HandleDirectiveCall(state);
                }
            }
            // Всё! Ничего не удаляем, парсер сам пропустит UNDEFINED
        }
	}

}