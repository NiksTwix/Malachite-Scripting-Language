#include "..\..\..\include\Common\Diagnostics\Diagnostics.hpp"
#include <iostream>


namespace MSLC 
{
    using namespace Diagnostics;



    void Logger::Print(const InformationMessage& message) const
    {
        switch (output_type)
        {
        case MSLC::Diagnostics::OutputType::CMD:
            PrintToCmd(message);
            break;
        case MSLC::Diagnostics::OutputType::External:
            for (auto handler : handlers) 
            {
                handler(message);
            }
            break;
        case MSLC::Diagnostics::OutputType::CMDAndExternal:
            PrintToCmd(message);
            for (auto handler : handlers)
            {
                handler(message);
            }
            break;
        default:
            break;
        }
    }

    output_handlerID Diagnostics::Logger::AddHandler(output_handler handler)
    {
        handlers.push_back(handler);

        return handlers.size();
    }

    bool Diagnostics::Logger::RemoveHandler(output_handlerID handler)
    {
        if (handler >= handlers.size()) return false;
        handlers.erase(handlers.begin() + handler);
        return true;
    }

    void Logger::PrintToCmd(const InformationMessage& message) const
    {

        std::string_view color = cmd_clear_style;

        switch (message.m_type)
        {
        case Info:
            break;
        case Success:
            color = cmd_success_style;
            break;
        case Warning:
            color = cmd_warning_style;
            break;
        case SyntaxError:
            color = cmd_syntax_error_style;
            break;
        case TypeError:
        case FileError:
        case LogicError:
            color = cmd_error_style;
            break;
        case DeveloperError:
            color = cmd_developer_error_style;
        default:
            break;
        }

        std::string place = "";

        switch (message.s_type)
        {
        case SourceCode:
            place = "Line:" + std::to_string(message.place);
            break;
        case IRCode:
            place = "Instruction pointer:" + std::to_string(message.place);
            break;
        default:
            break;
        }

        std::cout << color << message.text << place << cmd_clear_style << "\n";
    }


    std::string Logger::FormatString(const std::string& format, const std::vector<std::string>& args) const
    {
        std::string result;
        size_t arg_index = 0;

        for (size_t i = 0; i < format.size(); ++i)
        {
            if (format[i] == '%' && i + 1 < format.size())
            {
                char specifier = format[i + 1];

                switch (specifier)
                {
                case 's': // string
                case 'i': // integer
                case 'f': // floating value
                case 'b': // boolean
                    if (arg_index < args.size())
                    {
                        result += args[arg_index++];
                        ++i; // Skip specifier
                    }
                    else
                    {
                        // If arguments dont exist in necessary count, skip
                        result += format[i];
                    }
                    break;
                case '%': // screened percentage
                    result += '%';
                    ++i;
                    break;
                default:
                    //Undefined specifier - skip
                    result += format[i];
                    break;
                }
            }
            else
            {
                result += format[i];
            }
        }

        return result;
    }

}