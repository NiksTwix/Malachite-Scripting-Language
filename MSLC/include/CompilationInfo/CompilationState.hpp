#pragma once
#include "..\Preprocessing\MacrosInfo.hpp"
#include "Variables.hpp"
#include "Functions.hpp"
#include "Types.hpp"

namespace MSLC 
{
	namespace CompilationInfo
	{

		using SymbolID = size_t;

		enum class SymbolType 
		{
			Undefined,
			Variable,
			Type,
			Function,
		};

		struct Symbol 
		{
			SymbolType type;

			DescriptionID description_id = INVALID_ID;				//Type,Variable
			std::vector<DescriptionID> description_ids;	//Function overloadings
		};


		struct GlobalSymbolTable 
		{
		private:
			SymbolID global_symbol_id = START_COUNTER_ID;
			Types::TypeID global_type_id = START_COUNTER_ID;
			Functions::FunctionID global_function_id = START_COUNTER_ID;
			Variables::VariableID global_variable_id = START_COUNTER_ID;

			std::unordered_map<Types::TypeID, Types::TypeDescription> types_descriptions;
			std::unordered_map<Functions::FunctionID, Functions::FunctionDescription> functions_descriptions;
			std::unordered_map<Variables::VariableID, Variables::VariableDescription> variables_descriptions;


			std::unordered_map<SymbolID, Symbol> symbols;


		public:
			//Methods
		};


		class CompilationState 
		{

			Preprocessing::MacrosTable macros_table;
		public:
			Preprocessing::MacrosTable& GetMacrosTable();
		};


	}
}