#pragma once
#include "..\Preprocessing\MacrosInfo.hpp"
#include "Variables.hpp"
#include "Functions.hpp"
#include "Types.hpp"

namespace MSLC 
{
	namespace CompilationInfo
	{
		using NamespaceID = DescriptionID;
		enum class SymbolType 
		{
			Undefined,
			Variable,
			Type,
			Function,
			Namespace,
		};

		struct Symbol 
		{
			SymbolType type;

			DescriptionID description_id = INVALID_ID;				//Type,Variable,Functions,Namespaces
			std::vector<DescriptionID> description_ids;	//Function overloadings
		};
		struct VisibleFrame
		{
			LocalSymbolTable lsl;
		};

		struct LocalSymbolTable 
		{
		private:
			std::unordered_map<std::string, Symbol> symbols{};
		public:

			//access and another methods
		};

		struct GlobalSymbolTable 
		{
		private:
			Types::TypeID global_type_id = START_COUNTER_ID;
			Functions::FunctionID global_function_id = START_COUNTER_ID;
			Variables::VariableID global_variable_id = START_COUNTER_ID;
			NamespaceID global_namespace_id = START_COUNTER_ID;

			std::unordered_map<Types::TypeID, Types::TypeDescription> types_descriptions{};
			std::unordered_map<Functions::FunctionID, Functions::FunctionDescription> functions_descriptions{};
			std::unordered_map<Variables::VariableID, Variables::VariableDescription> variables_descriptions{};
			std::unordered_map<NamespaceID, VisibleFrame> namespaces{};

		public:

			//access and another methods
		};

		

		class CompilationState 
		{
			GlobalSymbolTable gst;
			//vf stack
			Preprocessing::MacrosTable macros_table;
		public:
			Preprocessing::MacrosTable& GetMacrosTable();
		};


	}
}