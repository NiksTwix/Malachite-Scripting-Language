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

			bool Has(const std::string& identifier) const { return symbols.count(identifier); }
			Symbol& Get(const std::string& identifier) { return symbols.at(identifier); }

			Symbol* GetSafe(const std::string& identifier) 
			{
				auto it = symbols.find(identifier);
				return it != symbols.end() ? &it->second : nullptr;
			}


			void Add(const std::string& identifier,Symbol symbol) { symbols[identifier] = symbol;}
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

			std::unordered_map<NamespaceID, std::shared_ptr<VisibleFrame>> namespaces{};		//just contains lsl as namespace

		public:

			NamespaceID AddNamespace(std::shared_ptr<VisibleFrame> frame);
			std::shared_ptr<VisibleFrame> GetNamespace(NamespaceID id);

			Types::TypeID AddType(Types::TypeDescription type_description);
			Types::TypeDescription* GetType(Types::TypeID id);
			bool HasType(Types::TypeID id);
			Variables::VariableID AddVariable(Variables::VariableDescription var_description);
			Variables::VariableDescription* GetVariable(Variables::VariableID id);
			bool HasVariable(Variables::VariableID id);


			//
			Types::TypeID GetLastTypeID() { return global_type_id - 1; }
			Variables::VariableID GetLastVariableID() { return global_variable_id - 1; }
			Functions::FunctionID GetLastFunctionID() { return global_function_id - 1; }
			NamespaceID GetLastNamespaceID() { return global_namespace_id - 1; }
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

		struct UnhandledSymbol
		{
			size_t global_id = 0;
			SymbolType symbol_type = SymbolType::Undefined;
			size_t desc_id = 0;
		};


		class CompilationState 
		{
			GlobalSymbolTable gst;

			size_t global_us_id = 0;

			Values::ImmediateConstantsTable ict;

			//vf stack
			std::vector<std::shared_ptr<VisibleFrame>> frames_stack;


			Preprocessing::MacrosTable macros_table;

			void InitBasics();
			std::unordered_map<size_t, UnhandledSymbol> unhandled_symbols;
		public:
			CompilationState();
			Preprocessing::MacrosTable& GetMacrosTable();
			GlobalSymbolTable& GetGST();
			Values::ImmediateConstantsTable& GetICT();

			Symbol* FindSymbolLocal(const std::string& identifier, bool check_parent = true);
			Values::ConstantID FindOrCreateConstant(const Definitions::ValueContainer& vcontainer);

			void PushNewFrame(const std::string& namespace_name);
			void PushNewFrame();	//pushes common frame

			void PopFrame();		

			//Registration

			Symbol* RegisterVariable(Variables::VariableDescription description);
			Symbol* RegisterFunction(Functions::FunctionDescription description);
			Symbol* RegisterType(Types::TypeDescription description);

			size_t AddUnhandledSymbol(SymbolType type, size_t desc_id);
			UnhandledSymbol* GetUnhandledSymbol(size_t global_id);
		};


	}
}