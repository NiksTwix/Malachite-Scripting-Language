#pragma once
#include "..\Tokenization\TokensInfo.hpp"


namespace MSLC
{

    using namespace Tokenization;

	namespace Preprocessing
	{
        using MacrosID = size_t;



        struct ParameterLabel 
        {
            size_t id = 0;
            std::string name;
        };

        enum class MacroType {
            Constant,
            FunctionLike
        };

        struct MacroDefinition {
            MacrosID macro_id;
            MacroType type;
            std::string name;

            // Для Constant
            Definitions::ValueContainer constant_value;

            // Для FunctionLike
            std::vector<Token> body_tokens;
            std::vector<ParameterLabel> parameters;

            // Минимальная отладочная информация
            size_t line_defined = 0;
            Definitions::ModuleId module_defined = 0;
        };

        class MacrosTable
        {
        private:
            std::unordered_map<MacrosID, MacroDefinition> m_macros;
            MacrosID m_next_id = 1;

            // Only for fast search in handling time
            std::unordered_map<std::string, MacrosID> m_name_to_id;

        public:
            // Minimum necessary interface

            MacrosID DefineConstant(const std::string& name,
                Definitions::ValueContainer value,
                Diagnostics::DebugInfo di);

            MacrosID DefineFunction(const std::string& name,
                std::vector<Token> body_tokens,
                std::vector<ParameterLabel> parameters,
                Diagnostics::DebugInfo di);

            // Main methods of fast access
            bool HasMacro(const std::string& name) const;

            MacroDefinition* TryGetMacro(const std::string& name);

            MacroDefinition& GetMacro(const std::string& name);

            MacroDefinition* TryGetMacro(MacrosID id);

            MacroDefinition& GetMacro(MacrosID id);

            // For clearing
            void Clear();

            // Stat (for debugging)
            size_t Size() const;
        };

	}
}