#include "..\..\include\CompilationInfo\CompilationState.hpp"

namespace MSLC
{
	namespace CompilationInfo
	{
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
		Symbol* CompilationState::FindSymbolLocal(const std::string& identifier)
		{
			if (frames_stack.empty()) return nullptr;

			for (size_t i = frames_stack.size() - 1; i >= 0; i--) 
			{
				VisibleFrame& frame = *frames_stack[i].get();
				if (frame.lsl.Has(identifier)) return &frame.lsl.Get(identifier);
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
			types_descriptions[type_id] = type_description;
			return type_id;
		}
		Types::TypeDescription* GlobalSymbolTable::GetType(Types::TypeID id)
		{
			auto it = types_descriptions.find(id);
			return it == types_descriptions.end() ? nullptr : &it->second;
		}
	}
}