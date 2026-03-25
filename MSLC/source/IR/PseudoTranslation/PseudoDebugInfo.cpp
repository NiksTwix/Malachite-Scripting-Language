#include "..\..\..\include\IR\PseudoTranslation\PseudoDebugInfo.hpp"



namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			using namespace CompilationInfo;
			std::string CompilationStateStringSerializator::SerializeValueInfoFlags(uint8_t flags)
			{
				std::string result;

				if (flags & Values::ValueFlags::ConstPointer) result += "ConstPointer ";
				if (flags & Values::ValueFlags::ConstValue) result += "ConstValue ";
				if (flags & Values::ValueFlags::Pointer) result += "Pointer ";
				if (flags & Values::ValueFlags::Reference) result += "Reference ";
				if (flags & Values::ValueFlags::Immediate) result += "Immediate ";
				if (flags & Values::ValueFlags::None) result = "None";
				return result;
			}
			std::string CompilationStateStringSerializator::Serialize(CompilationInfo::CompilationState& state)
			{
				std::stringstream ss;
				ss << "-CompilationState info\n";
				
				
				auto get_tabs = [](size_t count)	-> std::string
					{
						std::string result;
						result.reserve(count);
						for (size_t i = 0; i < count; i++) { result += '\t'; }
						return result;
					};
				ss << "--Types\n";
				for (size_t i = 0; i <= state.GetGST().GetLastTypeID(); i++) 
				{
					Types::TypeDescription* type_desc = state.GetGST().GetType(i);
					if (!type_desc) continue;
					ss << "Name:" << type_desc->name << get_tabs(tabs_count) << "ID:" << type_desc->id << get_tabs(tabs_count) << "Size:" << type_desc->size << "\n";
				}
				ss << "--Variables\n";
				for (size_t i = 0; i <= state.GetGST().GetLastVariableID(); i++)
				{
					Variables::VariableDescription* var_desc = state.GetGST().GetVariable(i);
					if (!var_desc) continue;
					ss << "+++++\n";
					ss << "Name:" << var_desc->name << get_tabs(tabs_count) << "ID:" << var_desc->id << "\n";
					ss << "ValueInfo\n";
					ss << "TypeID:" << var_desc->vinfo.type_id << get_tabs(tabs_count) << "PointersDepth:" << var_desc->vinfo.pointers_depth << get_tabs(tabs_count) << "Flags:" << SerializeValueInfoFlags(var_desc->vinfo.flags) << "\n";
				}
				ss << "--Constants\n";
				for (size_t i = 0; i < state.GetICT().constants_by_id.size(); i++)
				{
					Definitions::ValueContainer& vc = state.GetICT().constants_by_id[i];
					ss << "ID:" << i << get_tabs(tabs_count) << "Value:" << vc.ToString() << "\n";
				}
				//Functions and more detailed stats will be in the future

				return ss.str();
			}
			PseudoOperationsStringSerializator::PseudoOperationsStringSerializator()
			{
				codes =
				{
					{PseudoOpCode::Nop, "Nop"},

					// Arithmetic
					{PseudoOpCode::Add, "Add"},
					{PseudoOpCode::Subtract, "Subtract"},
					{PseudoOpCode::Divide, "Divide"},
					{PseudoOpCode::Multiply, "Multiply"},
					{PseudoOpCode::Mod, "Mod"},
					{PseudoOpCode::Negative, "Negative"},

					{PseudoOpCode::PrefixIncrement, "PrefixIncrement"},
					{PseudoOpCode::PrefixDecrement, "PrefixDecrement"},
					{PseudoOpCode::PostfixDecrement, "PostfixDecrement"},
					{PseudoOpCode::PostfixIncrement, "PostfixIncrement"},

					{PseudoOpCode::Exponentiate, "Exponentiate"},

					{PseudoOpCode::BitOffsetRight, "BitOffsetRight"},
					{PseudoOpCode::BitOffsetLeft, "BitOffsetLeft"},

					{PseudoOpCode::Assign, "Assign"},
					{PseudoOpCode::AssignR, "AssignR"},
					{PseudoOpCode::Push, "Push"},
					{PseudoOpCode::Pop, "Pop"},
					// Work with pointers
					{PseudoOpCode::GetPointer, "GetPointer"},
					{PseudoOpCode::Dereference, "Dereference"},

					{PseudoOpCode::GetFieldByValue, "GetFieldByValue"},
					{PseudoOpCode::GetFieldByPointer, "GetFieldByPointer"},

					// Logic
					{PseudoOpCode::Greater, "Greater"},
					{PseudoOpCode::Less, "Less"},
					{PseudoOpCode::GreaterEqual, "GreaterEqual"},
					{PseudoOpCode::LessEqual, "LessEqual"},
					{PseudoOpCode::Equal, "Equal"},
					{PseudoOpCode::NotEqual, "NotEqual"},

					{PseudoOpCode::And, "And"},
					{PseudoOpCode::Or, "Or"},
					{PseudoOpCode::Not, "Not"},
					{PseudoOpCode::BitAnd, "BitAnd"},
					{PseudoOpCode::BitOr, "BitOr"},
					{PseudoOpCode::BitNot, "BitNot"},

					{PseudoOpCode::DeclareVariable, "DeclareVariable"},
					{PseudoOpCode::DefineFunction, "DefineFunction"},

					{PseudoOpCode::Use, "Use"},
					{PseudoOpCode::UseConstant, "UseConstant"},
					{PseudoOpCode::UseType, "UseType"},
					{PseudoOpCode::Call, "Call"},
					{PseudoOpCode::CreateArray, "CreateArray"},

					{PseudoOpCode::PushFrame, "PushFrame"},
					{PseudoOpCode::PopFrame, "PopFrame"},
					{PseudoOpCode::PushLLOpers, "PushLLOpers"},
				};
			}
			std::string PseudoOperationsStringSerializator::Serialize(const PseudoOperation& operation)
			{
				auto get_tabs = [](size_t count)	-> std::string
					{
						std::string result;
						result.reserve(count);
						for (size_t i = 0; i < count; i++) { result += '\t'; }
						return result;
					};

				std::stringstream result;
				result << "OpCode:" << codes[operation.op_code] << get_tabs(tabs_count) << "Arg0:" << operation.arg_0 << get_tabs(tabs_count) << "Arg1:" << operation.arg_1 << get_tabs(tabs_count) << "Arg2:" << operation.arg_2 << get_tabs(tabs_count) << "Line:" << operation.debug_line << "\n";
				return result.str();
			}
			std::string PseudoOperationsStringSerializator::Serialize(POperationArray& array)
			{
				std::string result = "-Pseudocode\n";
				for (size_t i = 0; i < array.Size(); i++)
				{
					result+= Serialize(array[i]);
				}
				return result;
			}
		}
	}
}