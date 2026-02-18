#pragma once
#include <string>
#include <cstdint>
#include "..\Common\Strings\StringOperations.hpp"
#include "..\Common\Diagnostics\Diagnostics.hpp"

namespace MSLC 
{
	namespace Definitions 
	{
        constexpr double UPPER_BOUND_OF_NORMAL_ACCURACY = 1e10;
        constexpr double LOWER_BOUND_OF_NORMAL_ACCURACY = 1e-10;

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

            size_t GetDataSize() const
            {
                switch (type)
                {
                case MSLC::Definitions::ValueType::VOID:return 0;
                case MSLC::Definitions::ValueType::UINT: return sizeof(uint64_t);
                case MSLC::Definitions::ValueType::INT:return sizeof(int64_t);
                case MSLC::Definitions::ValueType::CHAR:return sizeof(char);
                case MSLC::Definitions::ValueType::BOOL:return sizeof(bool);
                case MSLC::Definitions::ValueType::REAL:return sizeof(double);
                case MSLC::Definitions::ValueType::STRING:
                    return strVal.size() * sizeof(char);
                default:
                    return 0;
                }
            }

            void* GetBytes() 
            {
                switch (type)
                {
                case MSLC::Definitions::ValueType::VOID:return nullptr;
                case MSLC::Definitions::ValueType::UINT: return &uintVal;
                case MSLC::Definitions::ValueType::INT:return &intVal;
                case MSLC::Definitions::ValueType::CHAR:return &charVal;
                case MSLC::Definitions::ValueType::BOOL:return &boolVal;
                case MSLC::Definitions::ValueType::REAL:return &realVal;
                case MSLC::Definitions::ValueType::STRING:
                    return strVal.data();

                default:
                    return 0;
                }
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

        class VCHash
        {
        public:
            size_t operator()(const ValueContainer& value_container) const
            {
                switch (value_container.type)
                {
                case ValueType::VOID:
                    return 0;
                case ValueType::INT:
                    return std::hash<int64_t>()(value_container.intVal);         
                case ValueType::BOOL:
                    return std::hash<bool>()(value_container.boolVal);
                case ValueType::REAL:
                    return std::hash<double>()(std::round(value_container.realVal * UPPER_BOUND_OF_NORMAL_ACCURACY) / UPPER_BOUND_OF_NORMAL_ACCURACY);
                case ValueType::CHAR:
                    return std::hash<char>()(value_container.charVal);
                case ValueType::UINT:
                    return std::hash<uint64_t>()(value_container.uintVal); 
                case ValueType::STRING:
                    return std::hash<std::string>()(value_container.strVal);
                
                }
                return 0;
            }
        };
        struct VCEqual
        {
            bool operator()(const ValueContainer& value_container0, const ValueContainer& value_container1) const
            {
                if (value_container0.type != value_container1.type) return false;
                switch (value_container0.type)
                {
                case ValueType::VOID:
                    return true;
                case ValueType::INT:
                    return value_container0.intVal == value_container1.intVal;
                case ValueType::BOOL:
                    return value_container0.boolVal == value_container1.boolVal;
                case ValueType::REAL:
                    return std::abs(value_container0.realVal - value_container1.realVal) < LOWER_BOUND_OF_NORMAL_ACCURACY;
                case ValueType::CHAR:
                    return value_container0.charVal == value_container1.charVal;
                case ValueType::UINT:
                    return value_container0.uintVal == value_container1.uintVal;
                case ValueType::STRING:
                    return value_container0.strVal == value_container1.strVal;

                }
                return false;
            }
        };
    
	}
}