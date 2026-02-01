#include "..\..\include\CompilationInfo\CompilationState.hpp"

namespace MSLC
{
	namespace CompilationInfo
	{
		void CompilationState::InitBasics()
		{
			//Creates first global frame;
			frames_stack.push_back(std::make_shared<VisibleFrame>());

			//basic types
			Symbol symbol;
			symbol.type = SymbolType::Type;
			//real
			Types::TypeDescription real_desc;
			real_desc.category = Types::TypeCategory::Primitive;
			real_desc.name = std::string(TypeMarkers::w_real);
			real_desc.primitive_analog = Types::Double;
			real_desc.size = 8;		
			symbol.description_id = gst.AddType(real_desc);
			frames_stack.back()->lsl.Add(real_desc.name, symbol);
			//int
			Types::TypeDescription int_desc;
			int_desc.category = Types::TypeCategory::Primitive;
			int_desc.name = std::string(TypeMarkers::w_integer);
			int_desc.primitive_analog = Types::Int;
			int_desc.size = 8;
			symbol.description_id = gst.AddType(int_desc);
			frames_stack.back()->lsl.Add(int_desc.name, symbol);

			//uint
			Types::TypeDescription uint_desc;
			uint_desc.category = Types::TypeCategory::Primitive;
			uint_desc.name = std::string(TypeMarkers::w_unsigned);
			uint_desc.primitive_analog = Types::UInt;
			uint_desc.size = 8;
			symbol.description_id = gst.AddType(uint_desc);
			frames_stack.back()->lsl.Add(uint_desc.name, symbol);
		}
		CompilationState::CompilationState()
		{
			InitBasics();
		}
		Preprocessing::MacrosTable& CompilationState::GetMacrosTable()
		{
			return macros_table;
		}
		GlobalSymbolTable& CompilationState::GetGST()
		{
			return gst;
		}
		Values::ImmediateConstantsTable& CompilationState::GetICT()
		{
			return ict;
		}
		Symbol* CompilationState::FindSymbolLocal(const std::string& identifier, bool check_parent)
		{
			if (frames_stack.empty()) return nullptr;

			if (check_parent) 
			{
				for (int i = frames_stack.size() - 1; i >= 0; i--)
				{
					VisibleFrame& frame = *frames_stack[i].get();
					if (frame.lsl.Has(identifier)) return &frame.lsl.Get(identifier);
				}
			}
			else 
			{
				auto frame = frames_stack.back();
				return frame->lsl.GetSafe(identifier);
			}


			return nullptr;
		}
		Values::ConstantID CompilationState::FindOrCreateConstant(const Definitions::ValueContainer& vcontainer)
		{
			return ict.GetOrAdd(vcontainer);
		}
		void CompilationState::PushNewFrame(const std::string& namespace_name)
		{
			std::shared_ptr<VisibleFrame> frame = std::make_shared<VisibleFrame>();

			frame->type = VisibleFrameType::NAMESPACE;
			

			Symbol symbol;
			symbol.description_id = gst.AddNamespace(frame);
			symbol.type = SymbolType::Namespace;
			if (!frames_stack.empty()) frames_stack.back()->lsl.Add(namespace_name, symbol);
			frames_stack.push_back(frame);
		}
		void CompilationState::PushNewFrame()
		{
			std::shared_ptr<VisibleFrame> frame = std::make_shared<VisibleFrame>();
			frames_stack.push_back(frame);
		}

		void CompilationState::PopFrame()
		{
			if (frames_stack.empty()) 
			{
				Diagnostics::Logger::Get().Print({ "PopFrame cannot be applied for empty frames stack.", Diagnostics::MessageType::DeveloperError, Diagnostics::SourceType::None });
				return;
			}
			frames_stack.pop_back();
		}

		Symbol* CompilationState::RegisterVariable(Variables::VariableDescription description)
		{
			Symbol symbol;
			symbol.type = SymbolType::Variable;
			symbol.description_id = GetGST().AddVariable(description);
			if (!frames_stack.empty())
			{
				frames_stack.back()->lsl.Add(description.name, symbol);
				return frames_stack.back()->lsl.GetSafe(description.name);
			}
			else return nullptr;
		}


		//GST
		NamespaceID GlobalSymbolTable::AddNamespace(std::shared_ptr<VisibleFrame> frame)
		{
			if (frame == nullptr) return INVALID_ID;
			size_t namespace_id = global_namespace_id++;
			namespaces[namespace_id] = frame;
			return namespace_id;
		}
		std::shared_ptr<VisibleFrame> GlobalSymbolTable::GetNamespace(NamespaceID id)
		{
			auto it = namespaces.find(id);
			return it == namespaces.end() ? nullptr: it->second;
		}
		Types::TypeID GlobalSymbolTable::AddType(Types::TypeDescription type_description)
		{
			size_t type_id = global_type_id++;
			type_description.id = type_id;
			types_descriptions[type_id] = type_description;
			return type_id;
		}
		Types::TypeDescription* GlobalSymbolTable::GetType(Types::TypeID id)
		{
			auto it = types_descriptions.find(id);
			return it == types_descriptions.end() ? nullptr : &it->second;
		}
		bool GlobalSymbolTable::HasType(Types::TypeID id)
		{
			return types_descriptions.count(id);
		}
		Variables::VariableID GlobalSymbolTable::AddVariable(Variables::VariableDescription var_description)
		{
			size_t var_id = global_variable_id++;
			var_description.id = var_id;
			variables_descriptions[var_id] = var_description;
			return var_id;
		}
		Variables::VariableDescription* GlobalSymbolTable::GetVariable(Variables::VariableID id)
		{
			auto it = variables_descriptions.find(id);
			return it == variables_descriptions.end() ? nullptr : &it->second;
		}
		bool GlobalSymbolTable::HasVariable(Variables::VariableID id)
		{
			return variables_descriptions.count(id);
		}
	}
}