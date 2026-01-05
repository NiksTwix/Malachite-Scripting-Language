#include "..\..\include\Preprocessing\MacrosInfo.hpp"
#include <stdexcept>


namespace MSLC
{

	namespace Preprocessing
	{
		MacrosID MacrosTable::DefineConstant(const std::string& name, Definitions::ValueContainer value, size_t line)
        {
            MacrosID id = m_next_id++;

            MacroDefinition md;
            md.macro_id = id;
            md.type = MacroType::Constant;
            md.name = name;
            md.constant_value = value;
            md.line_defined = line;

            m_macros[id] = md;
            m_name_to_id[name] = id;

            return id;
        }
        MacrosID MacrosTable::DefineFunction(const std::string& name, std::vector<Token> body_tokens, std::vector<std::string> parameters, size_t line)
        {
            MacrosID id = m_next_id++;

            MacroDefinition md;
            md.macro_id = id;
            md.type = MacroType::FunctionLike;
            md.name = name;
            md.body_tokens = std::move(body_tokens);
            md.parameters = std::move(parameters);
            md.line_defined = line;

            m_macros[id] = md;
            m_name_to_id[name] = id;

            return id;
        }
        bool MacrosTable::HasMacro(const std::string& name) const
        {
            return m_name_to_id.count(name);
        }
        MacroDefinition* MacrosTable::TryGetMacro(const std::string& name)
        {
            auto it = m_name_to_id.find(name);
            if (it == m_name_to_id.end()) return nullptr;
            return &m_macros[it->second];
        }
        MacroDefinition& MacrosTable::GetMacro(const std::string& name)
        {
            auto* macro = TryGetMacro(name);
            if (!macro) throw std::runtime_error("Macro not found: " + name);
            return *macro;
        }
        MacroDefinition* MacrosTable::TryGetMacro(MacrosID id)
        {
            auto it = m_macros.find(id);
            return it != m_macros.end() ? &it->second : nullptr;
        }
        MacroDefinition& MacrosTable::GetMacro(MacrosID id)
        {
            auto* macro = TryGetMacro(id);
            if (!macro) throw std::runtime_error("Macro ID not found: " + std::to_string(id));
            return *macro;
        }
        void MacrosTable::Clear()
        {
            m_macros.clear();
            m_name_to_id.clear();
            m_next_id = 1;
        }
        size_t MacrosTable::Size() const
        {
            return m_macros.size();
        }
	}
}