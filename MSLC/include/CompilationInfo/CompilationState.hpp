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
		

		


		struct LocalSymbolTable 
		{
		private:
			std::unordered_map<std::string, Symbol> symbols{};
		public:

			//access and another methods
		};


		struct VisibleFrame;

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

			std::vector<Definitions::ValueContainer> constants_pool{};

			std::unordered_map<NamespaceID, LocalSymbolTable> namespaces{};		//just contains lsl as namespace

		public:

			//access and another methods
		};

		enum class VisibleFrameType 
		{
			COMMON,
			NAMESPACE,
		};


		struct VisibleFrame
		{
			LocalSymbolTable lsl;
			VisibleFrameType type;	//If namespace - after handling lsl will be copied in global table
		};

		class CompilationState 
		{
			GlobalSymbolTable gst;

			Variables::ImmediateConstantsTable ict;

			//vf stack
			std::vector<VisibleFrame> frames_stack;


			Preprocessing::MacrosTable macros_table;

			

		public:
			Preprocessing::MacrosTable& GetMacrosTable();
			GlobalSymbolTable& GetGST();
			Variables::ImmediateConstantsTable& GetICT();

		};


	}
}