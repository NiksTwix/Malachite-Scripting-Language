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
		using LabelID = DescriptionID;
		enum class SymbolType : uint8_t
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

		struct UnhandledSymbol
		{
			size_t global_id = 0;
			SymbolType symbol_type = SymbolType::Undefined;
			size_t desc_id = 0;
			uint32_t module_id = 0;
		};

		struct pair_hash {
			template <class T1, class T2>
			std::size_t operator()(const std::pair<T1, T2>& p) const {
				auto h1 = std::hash<T1>{}(p.first);
				auto h2 = std::hash<T2>{}(p.second);
				return h1 ^ (h2 << 1);
			}
		};


		struct GlobalSymbolTable 
		{
		private:
			Types::TypeID global_type_id = START_COUNTER_ID;
			Functions::FunctionID global_function_id = START_COUNTER_ID;
			Variables::VariableID global_variable_id = START_COUNTER_ID;
			NamespaceID global_namespace_id = START_COUNTER_ID;
			LabelID global_label_id = START_COUNTER_ID;



			std::unordered_map<Types::TypeID, Types::TypeDescription> types_descriptions{};
			std::unordered_map<Functions::FunctionID, Functions::FunctionDescription> functions_descriptions{};
			std::unordered_map<Variables::VariableID, Variables::VariableDescription> variables_descriptions{};

			std::vector<Definitions::ValueContainer> constants_pool{};

			std::unordered_map<NamespaceID, std::shared_ptr<VisibleFrame>> namespaces{};		//just contains lsl as namespace
			

			//Unhandled Symbols
			std::unordered_map<std::pair<SymbolType, size_t>, size_t, pair_hash> local_desc_to_global_id;
			std::unordered_map<size_t, UnhandledSymbol> unhandled_symbols;
			size_t global_us_id = 0;

			//Labels
			std::unordered_map<std::string, CompilationInfo::LabelID> label_ids;
		public:

			NamespaceID AddNamespace(std::shared_ptr<VisibleFrame> frame);
			std::shared_ptr<VisibleFrame> GetNamespace(NamespaceID id);

			Types::TypeID AddType(Types::TypeDescription type_description);
			Types::TypeDescription* GetType(Types::TypeID id);
			bool HasType(Types::TypeID id);
			Variables::VariableID AddVariable(Variables::VariableDescription var_description);
			Variables::VariableDescription* GetVariable(Variables::VariableID id);
			bool HasVariable(Variables::VariableID id);

			Functions::FunctionID AddFunction(Functions::FunctionDescription fun_description);
			Functions::FunctionDescription* GetFunction(Functions::FunctionID id);
			bool HasFunction(Functions::FunctionID id);
			//
			Types::TypeID GetLastTypeID() { return global_type_id - 1; }
			Variables::VariableID GetLastVariableID() { return global_variable_id - 1; }
			Functions::FunctionID GetLastFunctionID() { return global_function_id - 1; }
			NamespaceID GetLastNamespaceID() { return global_namespace_id - 1; }


			size_t AddUnhandledSymbol(SymbolType type, size_t desc_id, moduleid current_module);
			UnhandledSymbol* GetUnhandledSymbol(size_t global_id);

			size_t GetUnhandledSymbolsCount() const;
			LabelID RegisterOrGetLabelID(std::string str_id);
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

		
		struct ConstantInfo 
		{
			size_t stack_offset;
			size_t using_count;
		};


		

		class CompilationState 
		{
			GlobalSymbolTable gst;

			Values::ImmediateConstantsTable ict;

			//vf stack
			std::vector<std::shared_ptr<VisibleFrame>> frames_stack;


			Preprocessing::MacrosTable macros_table;

			void InitBasics();

			std::unordered_map<Values::ConstantID, ConstantInfo> constants_info;

			uint32_t current_module_id = 0;

			uint16_t compilation_flags = 0;

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

			void SetModule(uint32_t module_) { current_module_id = module_; }

			//Registration

			Symbol* RegisterVariable(Variables::VariableDescription description);
			Symbol* RegisterFunction(Functions::FunctionDescription description);
			Symbol* RegisterType(Types::TypeDescription description);

			size_t AddUnhandledSymbol(SymbolType type, size_t desc_id);
			UnhandledSymbol* GetUnhandledSymbol(size_t global_id);

			size_t GetUnhandledSymbolsCount() const;

			std::unordered_map<Values::ConstantID, ConstantInfo>& ConstantInfoTable() { return constants_info; }


			inline uint16_t& GetCompilationFlags() {
				return compilation_flags;
			}
		};


	}
}