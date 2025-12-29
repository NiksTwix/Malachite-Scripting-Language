#pragma once
#include <string>
#include <cstdint>
#include "..\Common\Strings\StringOperations.hpp"
#include "..\Common\Diagnostics\Diagnostics.hpp"

namespace MSLC 
{
	namespace Definitions 
	{
		enum class ValueType 
		{
			VOID = 0,
			UINT,
			INT,
			CHAR,
			BOOL,
			REAL,
			STRING,
		};

        struct ValueContainer {
            ValueType type;

            union {
                double realVal;    //Real in Malachite is double 
                bool boolVal;
                int64_t intVal;
                uint64_t uintVal;
                char charVal;
            };
            std::string strVal;   // Для строк 
            // Конструкторы
            ValueContainer() : type(ValueType::VOID), realVal(0) {}

            ValueContainer(int64_t v, ValueType type) : type(type), intVal(v) {}

            ValueContainer(double v) : type(ValueType::REAL), realVal(v) {}
            ValueContainer(int64_t v) : type(ValueType::INT), intVal(v) {}
            ValueContainer(uint64_t v) : type(ValueType::UINT), uintVal(v) {}
            ValueContainer(bool v) : type(ValueType::BOOL), boolVal(v) {}
            ValueContainer(char v) : type(ValueType::CHAR), charVal(v) {}
            ValueContainer(const std::string& s) : type(ValueType::STRING) {
                new (&strVal) std::string(s);
            }

            ValueContainer(const char* s) : type(ValueType::STRING) {
                new (&strVal) std::string(s);
            }

            // ПРАВИЛО ПЯТИ (очень важно!)
            // Конструктор копирования
            ValueContainer(const ValueContainer& other) : type(other.type), uintVal(other.uintVal) {
                if (type == ValueType::STRING) new (&strVal) std::string(other.strVal);
            }

            // Оператор присваивания
            ValueContainer& operator=(const ValueContainer& other) {
                // Если текущий объект уже хранит строку, её нужно уничтожить
                if (this == &other) return *this; // Защита от самоприсваивания
                destroyCurrent();
                uintVal = other.uintVal;
                type = other.type;

                if (type == ValueType::STRING) new (&strVal) std::string(other.strVal);
                return *this;
            }
            ValueContainer(ValueContainer&& other) noexcept : type(other.type), uintVal(other.uintVal) {
                if (type == ValueType::STRING) new (&strVal) std::string(std::move(other.strVal));
                other.type = ValueType::VOID; // Чтобы деструктор other ничего не делал
            }

            // Оператор перемещающего присваивания
            ValueContainer& operator=(ValueContainer&& other) noexcept {
                if (this != &other) {
                    destroyCurrent(); // Уничтожаем текущие данные
                    type = other.type;
                    uintVal = other.uintVal;
                    if (type == ValueType::STRING) new (&strVal) std::string(std::move(other.strVal));
                    other.type = ValueType::VOID;
                }
                return *this;
            }

            std::string ToString() const
            {
                switch (type)
                {
                case ValueType::VOID:
                    return "";
                    break;
                case ValueType::INT:
                    return std::to_string(intVal);
                    break;
                case ValueType::UINT:
                    return std::to_string(uintVal);
                    break;
                case ValueType::BOOL:
                    return boolVal ? "true" : "false";
                    break;
                case ValueType::REAL:
                    return std::to_string(realVal);
                    break;
                case ValueType::STRING:
                    return strVal;
                    break;
                case ValueType::CHAR:
                    return std::to_string(charVal);
                    break;
                default:
                    return "";
                    break;
                }
            }
            static std::string GetTypeString(ValueType type)
            {
                switch (type)
                {
                case ValueType::VOID:
                    return "void";
                    break;
                case ValueType::INT:
                    return "int";
                    break;
                case ValueType::UINT:
                    return"uint";
                    break;
                case ValueType::BOOL:
                    return "bool";
                    break;
                case ValueType::REAL:
                    return "float";
                    break;
                case ValueType::STRING:
                    return "string";
                    break;
                case ValueType::CHAR:
                    return "char";
                    break;
                default:
                    return "undefined type";
                    break;
                }
            }
            // Деструктор
            ~ValueContainer() {
                destroyCurrent();
            }

        private:
            void destroyCurrent() {
                // Явно вызываем деструктор только для строки
                if (type == ValueType::STRING) {
                    strVal.~basic_string();
                }
                // Для shared_ptr и примитивов деструктор вызывать не нужно
            }
        };

        //WARNING: you must be care with this function
        ValueContainer CastTo(ValueContainer value, ValueType type, int line = -1);
	}
}