#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <typeinfo>
#include <stack>

namespace MSLC 
{
	namespace Diagnostics
	{
		namespace ErrorTexts 
		{
			constexpr std::string_view undefined_symbol = "Undefined symbol \"%s\" in current space.";
            constexpr std::string_view file_loading_bad_path = "File loading is failed on path \"%s\".";
            constexpr std::string_view import_directive_argument = "Bad argument of import directive: \"%s\". File: \"%s\".";
            constexpr std::string_view value_container_invalid_casting = "Casting %s to %s is invalid.";
		}
		constexpr std::string_view cmd_clear_style = "\033[0m";
		constexpr std::string_view cmd_error_style = "\033[31m";
		constexpr std::string_view cmd_warning_style = "\033[33m";
		constexpr std::string_view cmd_syntax_error_style = "\033[35m";
		constexpr std::string_view cmd_success_style = "\033[32m";

		enum MessageType
		{
			Info,
			Success,
			Warning,
			SyntaxError,
			TypeError,
			FileError,
			LogicError,
		};

		enum SourceType 
		{
            None,
			SourceCode,
			ByteCode
		};


		struct InformationMessage
		{
			std::string text;
			SourceType	s_type;
			MessageType m_type;
			size_t place;
			

			InformationMessage(const std::string& text, MessageType m_type, SourceType s_type, size_t place) : text(text), m_type(m_type), s_type(s_type), place(place) {}
			InformationMessage(const std::string& text, MessageType m_type, size_t place) : text(text), m_type(m_type), s_type(SourceType::SourceCode), place(place) {}
			InformationMessage(const std::string& text, size_t place) : text(text), m_type(MessageType::Info), s_type(SourceType::SourceCode), place(place) {}
			InformationMessage(const std::string& text) : text(text), m_type(MessageType::Info), s_type(SourceType::None), place(0) {}
		};
	
		


        class Logger
        {
        private:
            // Singleton паттерн
            Logger() = default;
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;

            // Function-helper for convertion to string 
            template<typename T>
            std::string GetString(const T& arg)
            {
                // Use SFINAE or if constexpr instead of static_assert с typeid
                if constexpr (std::is_same_v<T, std::string>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, const char*>) {
                    return std::string(arg);
                }
                else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
                    return std::to_string(arg);
                }
                else if constexpr (std::is_same_v<T, bool>) {
                    return arg ? "true" : "false";
                }
                else {
                    // For another type we can use stringstream 
                    std::ostringstream oss;
                    oss << arg;
                    return oss.str();
                }
            }

            // Specialization for strings literals
            std::string GetString(const char* arg)
            {
                return std::string(arg);
            }

        public:
            // Singleton getter
            static Logger& Get()
            {
                static Logger instance;
                return instance;
            }
 
            void PrintToCmd(const InformationMessage& message) const;

            // Main format function
            template<typename... Args>
            void PrintWithFormat(InformationMessage message, Args... args)
            {
                std::vector<std::string> args_strings;

                // Collect all arguments
                (args_strings.push_back(GetString(args)), ...);

                // FormatString
                std::string result = FormatString(message.text, args_strings);
                message.text = result;

                // Printing
                PrintToCmd(message);
            }

        private:
            // Function of string's format
            std::string FormatString(const std::string& format, const std::vector<std::string>& args) const;
           
        };

	}

}