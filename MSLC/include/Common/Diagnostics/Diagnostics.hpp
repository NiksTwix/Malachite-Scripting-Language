#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <typeinfo>
#include <stack>
#include <functional>
#include <sstream>

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
        constexpr std::string_view cmd_developer_error_style = "\033[3;36m";

		enum MessageType
		{
			Info,
			Success,
			Warning,
			SyntaxError,
			TypeError,
			FileError,
			LogicError,
            DeveloperError
		};

		enum SourceType 
		{
            None,
			SourceCode,
			IRCode,
		};

        struct DeclaringPlace 
        {
            uint32_t place = 0; //line or pseudo, byte instruction pointer 
            uint16_t module_id = 0;

            DeclaringPlace() = default;
            DeclaringPlace(uint32_t place, uint16_t module_id) : place(place), module_id(module_id) {}
        };


		struct InformationMessage
		{
			std::string text;
			SourceType	s_type;
			MessageType m_type;
			DeclaringPlace di;
			

			InformationMessage(const std::string& text, MessageType m_type, SourceType s_type, DeclaringPlace di) : text(text), m_type(m_type), s_type(s_type), di(di) {}
			InformationMessage(const std::string& text, MessageType m_type, DeclaringPlace di) : text(text), m_type(m_type), s_type(SourceType::SourceCode), di(di) {}
			InformationMessage(const std::string& text, DeclaringPlace di) : text(text), m_type(MessageType::Info), s_type(SourceType::SourceCode), di(di) {}
            InformationMessage(const std::string& text, MessageType m_type) : text(text), m_type(m_type), s_type(SourceType::SourceCode) {}
			InformationMessage(const std::string& text) : text(text), m_type(MessageType::Info), s_type(SourceType::None) {}
		};
	

        using output_handler = std::function<void(InformationMessage message)>;
        using output_handlerID = size_t;

        enum class OutputType 
        {
            CMD,
            External,
            CMDAndExternal,
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

            OutputType output_type = OutputType::CMD;

            std::vector<output_handler> handlers;

        public:
            // Singleton getter
            static Logger& Get()
            {
                static Logger instance;
                return instance;
            }
 
            void Print(const InformationMessage& message) const;

            output_handlerID AddHandler(output_handler handler);

            bool RemoveHandler(output_handlerID handler);

            void SetOutputMode(OutputType output_type) { this->output_type = output_type; }


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
                Print(message);
            }

        private:
            // Function of string's format
            std::string FormatString(const std::string& format, const std::vector<std::string>& args) const;
            void PrintToCmd(const InformationMessage& message) const;
        };

	}

}