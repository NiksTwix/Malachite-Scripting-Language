#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <filesystem>

namespace MSLC
{
    namespace Strings
    {
        class StringOperations
        {
        public:
            static std::vector<std::string> SplitString(const std::string& str, char delimiter, int limit = -1) {
                std::vector<std::string> tokens;
                try
                {
                    if (str.empty()) return tokens;
                    if (limit == 0) return tokens;

                    size_t start = 0;
                    size_t end = str.find(delimiter);
                    size_t count = 0;

                    while (end != std::string::npos)
                    {
                        // Безопасное извлечение подстроки
                        if (start < str.length()) {
                            tokens.push_back(str.substr(start, end - start));
                            if (limit > 0 && ++count >= static_cast<size_t>(limit)) {
                                tokens.push_back(str.substr(end + 1));
                                return tokens;
                            }
                        }
                        start = end + 1;
                        end = str.find(delimiter, start);
                    }

                    // Добавляем остаток строки (последний токен)
                    if (start < str.length()) {
                        tokens.push_back(str.substr(start));
                    }
                }
                catch (std::exception e) {
                    std::cout << "SplitString error:" << e.what() << "\n";
                    std::cout << "String is " << str << "\n";
                }

                return tokens;
            }
            static std::string EraseString(const std::string& str, const std::string& sub_str) {
                // 1. Проверка тривиальных случаев
                if (sub_str.empty() || str.empty() || sub_str.length() > str.length()) {
                    return str;
                }

                std::string result;
                result.reserve(str.length()); // Оптимизация памяти

                size_t last_pos = 0;
                size_t pos = str.find(sub_str);

                // 2. Основной цикл удаления
                while (pos != std::string::npos) {
                    // Добавляем часть перед подстрокой (если есть)
                    if (pos > last_pos) {
                        result.append(str, last_pos, pos - last_pos);
                    }

                    last_pos = pos + sub_str.length();
                    pos = str.find(sub_str, last_pos);
                }

                // 3. Добавляем остаток строки
                if (last_pos < str.length()) {
                    result.append(str, last_pos, str.length() - last_pos);
                }

                // 4. Удаляем возможные пробелы в начале/конце
                auto trim = [](std::string& s) {
                    s.erase(0, s.find_first_not_of(" \t\n\r"));
                    s.erase(s.find_last_not_of(" \t\n\r") + 1);
                    };

                trim(result);
                return result;
            }

            static bool IsNumber(const std::string& s) {
                if (s.empty()) return false;
                size_t start = (s[0] == '-') ? 1 : 0;
                bool has_dot = false;
                for (size_t i = start; i < s.size(); ++i) {
                    if (s[i] == '.') {
                        if (has_dot) return false; // Две точки в числе
                        has_dot = true;
                    }
                    else if (!std::isdigit(s[i])) {
                        return false;
                    }
                }
                return true;
            }
            template<typename T>
            static std::vector<T> TrimVector(const std::vector<T>& vector, int start_index, int end_index)
            {
                std::vector<T> result;
                for (int i = start_index; i <= end_index; i++)
                {
                    result.push_back(vector[i]);
                }
                return result;
            }

            static std::string GenerateLabel(const std::string& base) {
                static int counter = 0;
                return base + "_" + std::to_string(counter++);
            }
        };
    }
}