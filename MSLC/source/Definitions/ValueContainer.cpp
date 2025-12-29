#pragma once
#include <string>
#include <cstdint>
#include "..\..\include\Definitions\ValueContainer.hpp"

namespace MSLC 
{
	namespace Definitions 
	{
		ValueContainer CastTo(ValueContainer value, ValueType type, int line)
        {
            switch (type)
            {
            case ValueType::VOID:
                return ValueContainer();
            case ValueType::INT:
                if (value.type == ValueType::STRING)
                {
                    if (!Strings::StringOperations::IsNumber(value.strVal))
                    {
                        Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage(Diagnostics::ErrorTexts::value_container_invalid_casting.data(), Diagnostics::MessageType::TypeError, line == -1 ? Diagnostics::None : Diagnostics::SourceCode, line), "string", "integer");
                        return ValueContainer();
                    }
                    return std::stoll(value.strVal);
                }
                if (value.type == ValueType::REAL) return static_cast<int64_t>(value.realVal);
                break;
            case ValueType::UINT:
                if (value.type == ValueType::STRING)
                {
                    if (!Strings::StringOperations::IsNumber(value.strVal))
                    {
                        Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage(Diagnostics::ErrorTexts::value_container_invalid_casting.data(), Diagnostics::MessageType::TypeError, line == -1 ? Diagnostics::None : Diagnostics::SourceCode, line), "string", "unsigned integer");
                        return ValueContainer();
                    }
                    return std::stoull(value.strVal);
                }
                if (value.type == ValueType::REAL) return static_cast<uint64_t>(value.realVal);
                break;
            case ValueType::BOOL:
                if (value.type == ValueType::STRING)
                {
                    if (value.strVal != "true" && value.strVal != "false")
                    {
                        Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage(Diagnostics::ErrorTexts::value_container_invalid_casting.data(), Diagnostics::MessageType::TypeError, line == -1 ? Diagnostics::None : Diagnostics::SourceCode, line), "string", "bool");
                        return ValueContainer();
                        return ValueContainer();
                    }
                    return value.strVal == "true" ? true : false;
                }
                if (value.type == ValueType::INT) return ValueContainer(static_cast<bool>(value.intVal));
                if (value.type == ValueType::UINT) return ValueContainer(static_cast<bool>(value.uintVal));
                if (value.type == ValueType::CHAR) return ValueContainer(static_cast<bool>(value.charVal));
                if (value.type == ValueType::REAL) return static_cast<bool>(value.realVal);
                break;
            case ValueType::REAL:
                if (value.type == ValueType::STRING)
                {
                    if (!Strings::StringOperations::IsNumber(value.strVal))
                    {
                        Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage(Diagnostics::ErrorTexts::value_container_invalid_casting.data(), Diagnostics::MessageType::TypeError, line == -1 ? Diagnostics::None : Diagnostics::SourceCode, line), "string", "real");
                        return ValueContainer();
                    }
                    return std::stod(value.strVal);
                }
                if (value.type == ValueType::INT) return static_cast<double>(value.intVal);
                if (value.type == ValueType::UINT) return static_cast<double>(value.uintVal);
                if (value.type == ValueType::BOOL) return static_cast<double>(value.boolVal);
                if (value.type == ValueType::CHAR) return static_cast<double>(value.charVal);
                break;
            case ValueType::STRING:
                if (value.type == ValueType::STRING)
                {
                    return value.strVal;
                }
                if (value.type == ValueType::INT) return std::to_string(value.intVal);
                if (value.type == ValueType::UINT) return std::to_string(value.uintVal);
                if (value.type == ValueType::BOOL) return std::to_string(value.boolVal);
                if (value.type == ValueType::REAL) return std::to_string(value.realVal);
                if (value.type == ValueType::CHAR) return std::to_string(value.charVal);
                break;
            case ValueType::CHAR:
                if (value.type == ValueType::STRING)
                {
                    if (value.strVal.size() != 1)
                    {
                        Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage(Diagnostics::ErrorTexts::value_container_invalid_casting.data(), Diagnostics::MessageType::TypeError, line == -1 ? Diagnostics::None : Diagnostics::SourceCode, line), "string", "char");
                        return ValueContainer();
                    }
                    return value.strVal[0];
                }
                if (value.type == ValueType::REAL) return (char)value.realVal;
                break;
            default:
                Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage(Diagnostics::ErrorTexts::value_container_invalid_casting.data(), Diagnostics::MessageType::TypeError, line == -1 ? Diagnostics::None : Diagnostics::SourceCode, line), "", "undefined type");
                return ValueContainer();
                break;
            }
            return ValueContainer(value.intVal, type);
        }
	}
}