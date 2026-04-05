#pragma once
#include "..\..\..\include\Translators\MSLVM_1\MSLVM_1Translator.hpp"

#include <string>

namespace MSLL
{
	namespace MSLVM_1
	{
		bool TranslatorVM_1::HandleCommand(std::shared_ptr<LinkDefinitions::CommandsPool> commands, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result, size_t& i)
		{

			LinkDefinitions::ByteCommand& cmd = commands->commands[i];

			//Symbol handling

			//if (cmd.flags & LinkDefinitions::Flag::UnhandledSymbol) 
			//{
			//	//checking arg0,arg1,arg2 -> replace symbol to address or ip by symbol's type
			//	
			//}
			auto checking = [](LinkDefinitions::CommandArgument& arg, std::shared_ptr<LinkDefinitions::LinkingState> state, size_t i, LinkDefinitions::moduleid m_id) -> void
				{
					if (arg.type == LinkDefinitions::CommandSource::Symbol)
					{
						if (arg.data >= state->symbols.size())
						{
							std::cerr << "Invalid symbol in argument 0. Byte Command index:" << i << "\n";

						}
						auto& symbol_info = state->symbols[arg.data];
						if (symbol_info.type == LinkDefinitions::SymbolType::Variable)
						{
							arg.data = state->stack_offset_of_module[symbol_info.module_id] + symbol_info.offset_m_c;
							arg.type = LinkDefinitions::CommandSource::MemoryAddress;
						}
						else if (symbol_info.type == LinkDefinitions::SymbolType::Function)
						{
							arg.data = symbol_info.native_code_offset;
							arg.type = LinkDefinitions::CommandSource::Immediate;
						}
					}
					else if (arg.type == LinkDefinitions::CommandSource::MemoryAddress)
					{
						arg.data = state->stack_offset_of_module[m_id] + arg.data;
					}
					else if (arg.type == LinkDefinitions::CommandSource::Constant)
					{
						arg.data = state->constants[arg.data].memory_offset;
						arg.type = LinkDefinitions::CommandSource::MemoryAddress;
					}
				};
			checking(cmd.arg0, state, i,commands->m_id);
			checking(cmd.arg1, state, i,commands->m_id);
			checking(cmd.arg2, state, i,commands->m_id);
			bool check = true;

			if ((cmd.code > LinkDefinitions::ByteOpCode::SECTION_ARITHMETIC_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_ARITHMETIC_ED) 
				|| (cmd.code > LinkDefinitions::ByteOpCode::SECTION_LOGIC_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_LOGIC_ED))
			{
				check = check && HandleAL(cmd, state, result);
			}
			else if ((cmd.code > LinkDefinitions::ByteOpCode::SECTION_COMPARING_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_COMPARING_ED))
			{
				check = check && HandleCOMP(cmd, state, result);
			}
			else if (cmd.code > LinkDefinitions::ByteOpCode::SECTION_CONTROL_FLOW_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_CONTROL_FLOW_ED)
			{
				check = check && HandleCF(cmd, state, result);
			}
			else if (cmd.code > LinkDefinitions::ByteOpCode::SECTION_MEMORY_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_MEMORY_ED)
			{
				check = check && HandleMR(cmd, state, result);
			}
			else if (cmd.code > LinkDefinitions::ByteOpCode::SECTION_SPECIAL_ST && cmd.code < LinkDefinitions::ByteOpCode::SECTION_SPECIAL_ED)
			{
				check = check && HandleSP(cmd, state, result);
			}
			else 
			{
				switch (cmd.code)
				{
				case LinkDefinitions::ByteOpCode::NOP:
					std::cerr << "Not a operation. Byte code instruction's number:" << i << "\n";
					break;
				default:
				{
					std::cerr << "Unsupported operation. Byte operation code:" << (int)cmd.code << "\n";
					return false;
					
				}
					break;
				}
			}

			return check;
		}

		bool TranslatorVM_1::HandleAL(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result)
		{
			VMOperationCode code = OpCodeTable::Get().At(command.code);
			if (code == VMOperationCode::NOP) 
			{
				std::cerr << "Unsupported operation. Byte operation code:" << (uint8_t)command.code << "\n";
				return false;
			}
		

			//Direct order of arguments
			PushVMCommand(code, command.arg0, command.arg1, command.arg2, result);
			return true;
		}

		bool TranslatorVM_1::HandleCOMP(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result)
		{
			size_t dest = command.arg0.data;
			size_t src0 = command.arg1.data;
			size_t src1 = command.arg2.data;

			VMOperation cmp_operation;

			cmp_operation.arg0 = src0;
			cmp_operation.arg1 = src1;

			VMOperation flag_operation;

			flag_operation.code = MSLVM_1::GET_FLAG;
			flag_operation.arg0 = dest;
			flag_operation.arg1 = MSLVM_1::Flag::NONE;
			flag_operation.arg2 = MSLVM_1::Flag::NONE;

			bool use_not = false;
			VMOperation not_operation;	
			not_operation.code = MSLVM_1::NOT_RR;
			not_operation.arg0 = dest;
			not_operation.arg1 = dest;
			//Flag's numbers will be defined in switch-case block

			switch (command.code)
			{
			case LinkDefinitions::ByteOpCode::EQUALI:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_INTEGER;
				flag_operation.arg1 = MSLVM_1::Flag::ZERO;
			}
				break;
			case LinkDefinitions::ByteOpCode::NOT_EQUALI:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_INTEGER;
				flag_operation.arg1 = MSLVM_1::Flag::ZERO;
				use_not = true;
			}
				break;
			case LinkDefinitions::ByteOpCode::GREATERI:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_INTEGER;
				flag_operation.arg1 = MSLVM_1::Flag::GREATER;
			}
				break;
			case LinkDefinitions::ByteOpCode::LESSI:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_INTEGER;
				flag_operation.arg1 = MSLVM_1::Flag::LESS;
			}
				break;
			case LinkDefinitions::ByteOpCode::EGREATERI:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_INTEGER;
				flag_operation.arg1 = MSLVM_1::Flag::GREATER;
				flag_operation.arg2 = MSLVM_1::Flag::ZERO;
			}
				break;
			case LinkDefinitions::ByteOpCode::ELESSI:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_INTEGER;
				flag_operation.arg1 = MSLVM_1::Flag::LESS;
				flag_operation.arg2 = MSLVM_1::Flag::ZERO;
			}
				break;
			case LinkDefinitions::ByteOpCode::EQUALU:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_UNSIGNED;
				flag_operation.arg1 = MSLVM_1::Flag::ZERO;
			}
				break;
			case LinkDefinitions::ByteOpCode::NOT_EQUALU:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_UNSIGNED;
				flag_operation.arg1 = MSLVM_1::Flag::ZERO;
				use_not = true;
			}
				break;
			case LinkDefinitions::ByteOpCode::GREATERU:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_UNSIGNED;
				flag_operation.arg1 = MSLVM_1::Flag::GREATER;
			}
				break;
			case LinkDefinitions::ByteOpCode::LESSU:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_UNSIGNED;
				flag_operation.arg1 = MSLVM_1::Flag::LESS;
			}
				break;
			case LinkDefinitions::ByteOpCode::EGREATERU:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_UNSIGNED;
				flag_operation.arg1 = MSLVM_1::Flag::GREATER;
				flag_operation.arg2 = MSLVM_1::Flag::ZERO;
			}
				break;
			case LinkDefinitions::ByteOpCode::ELESSU:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_UNSIGNED;
				flag_operation.arg1 = MSLVM_1::Flag::LESS;
				flag_operation.arg2 = MSLVM_1::Flag::ZERO;
			}
				break;
			case LinkDefinitions::ByteOpCode::EQUALR:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_REAL;
				flag_operation.arg1 = MSLVM_1::Flag::ZERO;
			}
			break;
			case LinkDefinitions::ByteOpCode::NOT_EQUALR:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_REAL;
				flag_operation.arg1 = MSLVM_1::Flag::ZERO;
				use_not = true;
			}
				break;
			case LinkDefinitions::ByteOpCode::GREATERR:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_REAL;
				flag_operation.arg1 = MSLVM_1::Flag::GREATER;
			}
				break;
			case LinkDefinitions::ByteOpCode::LESSR:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_REAL;
				flag_operation.arg1 = MSLVM_1::Flag::LESS;
			}
				break;
			case LinkDefinitions::ByteOpCode::EGREATERR:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_REAL;
				flag_operation.arg1 = MSLVM_1::Flag::GREATER;
				flag_operation.arg2 = MSLVM_1::Flag::ZERO;
			}
				break;
			case LinkDefinitions::ByteOpCode::ELESSR:
			{
				cmp_operation.code = MSLVM_1::CMP_RR_REAL;
				flag_operation.arg1 = MSLVM_1::Flag::LESS;
				flag_operation.arg2 = MSLVM_1::Flag::ZERO;
			}
				break;
			default:
				std::cerr << "Unsupported operation. Byte operation code:" << (int)command.code << "\n";
				return false;
			}

			result.push_back(cmp_operation);
			result.push_back(flag_operation);
			if (use_not) result.push_back(not_operation);
			return true;
		}

		bool TranslatorVM_1::HandleCF(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result)
		{
			VMOperation operation;
			switch (command.code)
			{
			case LinkDefinitions::ByteOpCode::JMP_LABEL:
			{
				LinkDefinitions::jmplabelid id = command.arg0.data;
				size_t current_ip = result.size();

				auto it = state->except_handling_jmps.find(id);
				if (it != state->except_handling_jmps.end()) 
				{
					for (size_t ip : it->second) 
					{
						result[ip].arg0 = current_ip;	//Sets currect jump id
					}
					state->except_handling_jmps.erase(id);
				}
				state->label_jmp_ip[id] = current_ip;

				return true;
			}
				break;
			case LinkDefinitions::ByteOpCode::JMP:
			{
				LinkDefinitions::jmplabelid id = command.arg0.data;
				size_t current_ip = result.size();
				operation.code = VMOperationCode::JMP;
				if (auto it = state->label_jmp_ip.find(id); it != state->label_jmp_ip.end())
				{
					operation.arg0 = it->second;
				
				}
				else 
				{
					state->except_handling_jmps[id].push_back(current_ip);
				}
			}
				break;
			case LinkDefinitions::ByteOpCode::JMPCV:
				break;
			case LinkDefinitions::ByteOpCode::JMPCNV:
				break;
			case LinkDefinitions::ByteOpCode::CALL:
				break;
			case LinkDefinitions::ByteOpCode::RET:
				break;
			case LinkDefinitions::ByteOpCode::EXT:
				break;
			default:
				break;
			}
			result.push_back(operation);
			return true;
		}

		bool TranslatorVM_1::HandleMR(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result)
		{
			switch (command.code)
			{
			case LinkDefinitions::ByteOpCode::MOVRR:
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RR;
				operation.arg0 = command.arg0.data;
				operation.arg1 = command.arg1.data;
				result.push_back(operation);
			}			
				break;
			case LinkDefinitions::ByteOpCode::MOVRI:
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = command.arg0.data;
				operation.arg1 = command.arg1.data;
				result.push_back(operation);
			}
				break;
			case LinkDefinitions::ByteOpCode::STACK_UP://Direct order of arguments
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = SpecialRegister::A0;
				operation.arg1 = command.arg0.data;
				result.push_back(operation);
				operation.code = VMOperationCode::ADD_RRR_UNSIGNED;
				operation.arg0 = SpecialRegister::SP;
				operation.arg1 = SpecialRegister::SP;
				operation.arg2 = SpecialRegister::A0;
				result.push_back(operation);
			}
				break;
			case LinkDefinitions::ByteOpCode::STACK_DOWN:
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = SpecialRegister::A0;
				operation.arg1 = command.arg0.data;
				result.push_back(operation);
				operation.code = VMOperationCode::SUB_RRR_UNSIGNED;
				operation.arg0 = SpecialRegister::SP;
				operation.arg1 = SpecialRegister::SP;
				operation.arg2 = SpecialRegister::A0;
				result.push_back(operation);
			}
				break;
			case LinkDefinitions::ByteOpCode::LEA_STATIC:
			{
				VMOperation operation;
				operation.code = VMOperationCode::MOV_RI;
				operation.arg0 = command.arg1.data;
				operation.arg1 = command.arg0.data;
				result.push_back(operation);

				break;
			}	
			case LinkDefinitions::ByteOpCode::LEA_DYNAMIC:
				std::cerr << "Unsupported operation. Byte operation code:" << (int)command.code << "\n";
				return false;

			case LinkDefinitions::ByteOpCode::LOAD_DYNAMIC:
			{
				PushVMCommand(VMOperationCode::LOAD_BY_ADDRESS, command.arg1, command.arg0, command.arg2, result);
				break;
			}	
			case LinkDefinitions::ByteOpCode::STORE_DYNAMIC:
			{
				PushVMCommand(VMOperationCode::STORE_BY_ADDRESS, command.arg1, command.arg0, command.arg2, result);
				break;
			}
			case LinkDefinitions::ByteOpCode::LOAD_CONST_STATIC:
			{
				PushVMCommand(VMOperationCode::LOAD_ABSOLUTE, command.arg1, command.arg0, command.arg2, result);//REGISTER FOR SAVING,ADDRESS IN ROD,SIZE
				break;
			}
			case LinkDefinitions::ByteOpCode::LEA_CONST:
			{
				std::cerr << "Unsupported operation. Byte operation code:" << (int)command.code << "\n";
				return false;
			}
			case LinkDefinitions::ByteOpCode::LOAD_STATIC:
			{
				PushVMCommand(VMOperationCode::LOAD_LOCAL, command.arg1, command.arg0, command.arg2, result);
				break;
			}
			case LinkDefinitions::ByteOpCode::STORE_STATIC:
			{
				PushVMCommand(VMOperationCode::STORE_LOCAL, command.arg1, command.arg0, command.arg2,result);
				break;
			}
			case LinkDefinitions::ByteOpCode::ALLOC:
			{
				PushVMCommand(VMOperationCode::ALLOCATE_MEMORY, command.arg0, command.arg1, command.arg2, result);
				break;
			}
			case LinkDefinitions::ByteOpCode::FREE:
			{
				PushVMCommand(VMOperationCode::FREE_MEMORY, command.arg0, command.arg1, command.arg2, result);
				break;
			}
			default:
				std::cerr << "Unsupported operation. Byte operation code:" << (int)command.code << "\n";
				break;
			}
			return true;
		}

		bool TranslatorVM_1::HandleSP(LinkDefinitions::ByteCommand command, std::shared_ptr<LinkDefinitions::LinkingState> state, std::vector<VMOperation>& result)
		{
			switch (command.code)
			{
			case LinkDefinitions::ByteOpCode::TC_ITR:
			{
				VMOperation operation;
				operation.code = VMOperationCode::TC_ITR_R;
				operation.arg0 = command.arg0.data;
				result.push_back(operation);
				break;
			}
				
			case LinkDefinitions::ByteOpCode::TC_RTI:
			{
				VMOperation operation;
				operation.code = VMOperationCode::TC_RTI_R;
				operation.arg0 = command.arg0.data;
				result.push_back(operation);
				break;
			}
			case LinkDefinitions::ByteOpCode::TC_UTR:
			{
				VMOperation operation;
				operation.code = VMOperationCode::TC_UTR_R;
				operation.arg0 = command.arg0.data;
				result.push_back(operation);
				break;
			}
			case LinkDefinitions::ByteOpCode::TC_UTI:
			{
				VMOperation operation;
				operation.code = VMOperationCode::TC_UTI_R;
				operation.arg0 = command.arg0.data;
				result.push_back(operation);
				break;
			};
			case LinkDefinitions::ByteOpCode::TC_RTU:
			{
				VMOperation operation;
				operation.code = VMOperationCode::TC_RTU_R;
				operation.arg0 = command.arg0.data;
				result.push_back(operation);
				break;
			}
			case LinkDefinitions::ByteOpCode::TC_ITU:
			{
				VMOperation operation;
				operation.code = VMOperationCode::TC_ITU_R;
				operation.arg0 = command.arg0.data;
				result.push_back(operation);
				break;
			}
			case LinkDefinitions::ByteOpCode::SPEC_CALL:
			{
				VMOperation operation;
				operation.code = VMOperationCode::VM_CALL;
				operation.arg0 = command.arg0.data;
				operation.arg1 = command.arg1.data;
				operation.arg2 = command.arg2.data;
				result.push_back(operation);
				break;
			}
			default:
				std::cerr << "Unsupported operation. Byte operation code:" << (int)command.code << "\n";
				return false;
			}
			return true;
		}

		void TranslatorVM_1::PushVMCommand(VMOperationCode code, LinkDefinitions::CommandArgument arg0, LinkDefinitions::CommandArgument arg1, LinkDefinitions::CommandArgument arg2, std::vector<VMOperation>& result)
		{
			VMOperation operation;
			operation.code = code;
			operation.arg0 = arg0.data;
			operation.arg1 = arg1.data;
			operation.arg2 = arg2.data;
			result.push_back(operation);
		}
		LinkDefinitions::ExecutionData TranslatorVM_1::Translate(fs::path directory, std::shared_ptr<LinkDefinitions::LinkingState> state, ObjectsReader& reader)
		{



			LinkDefinitions::ExecutionData execution_data;

			execution_data.read_only_data.allocate(state->constants_size);

			//moving data, constants freeing and offset calculating

			size_t rod_offset = 0;

			for (auto& constant: state->constants)
			{
				constant.memory_offset = rod_offset;

				if (constant.real_size_in_bytes + rod_offset > execution_data.read_only_data.bytes_size) 
				{
					std::cerr << "Invalid size calculating in the constants handling process.";
					execution_data.Free();
					state->FreeConstants();
					return LinkDefinitions::ExecutionData();
				}

				memcpy(execution_data.read_only_data.ptr + rod_offset, constant.data, constant.real_size_in_bytes); 
				rod_offset += constant.using_size_in_bytes;
				

				constant.Free();
			}

			

			rod_offset = ((rod_offset + DEFAULT_ALIGNMENT - 1) / DEFAULT_ALIGNMENT) * DEFAULT_ALIGNMENT;
			state->rod_offset_aligned = rod_offset;
			state->global_memory_offset = rod_offset;

			execution_data.aligned_rod_size = rod_offset;

			std::vector<VMOperation> commands;
			for (LinkDefinitions::moduleid id : state->linking_order) 
			{
				std::string file_name = state->module_prefix + std::to_string(id) + "." + state->module_extention;
				state->stack_offset_of_module[id] = state->global_memory_offset;
				bool check = HandleModule(directory / file_name, state, reader, commands,id);
				if (!check) 
				{
					std::cerr << "Error of module(" << id << ") handling. Module's name:" << file_name << "\n";
					execution_data.read_only_data.release();
					return LinkDefinitions::ExecutionData();
				}
			}

			execution_data.code.allocate(commands.size() * sizeof(VMOperation));

			::memcpy(execution_data.code.ptr,commands.data(), commands.size() * sizeof(VMOperation));

			if (!state->except_handling_jmps.empty()) 
			{
				std::cerr << "The source code contains unhandled jump commands.\n";
				execution_data.Free();
				return LinkDefinitions::ExecutionData();
			}


			return execution_data;
		}
		bool TranslatorVM_1::HandleModule(fs::path file, std::shared_ptr<LinkDefinitions::LinkingState> state, ObjectsReader& reader, std::vector<VMOperation>& commands, LinkDefinitions::moduleid id)
		{
			LinkDefinitions::static_bpointer  co_bytes = reader.ReadFile(file.string());
			if (co_bytes.ptr == nullptr)
			{
				std::cerr << "Invalid CO file \"" + file.string() + "\".\n";
				return false;
			}

			std::shared_ptr<LinkDefinitions::CommandsPool> pool = reader.DeserializeCO(co_bytes);

			pool->m_id = id;

			for (size_t i = 0; i < pool->commands.size(); i++) 
			{
				if (!HandleCommand(pool, state, commands, i)) return false;
			}

			state->global_memory_offset += pool->stack_size;
			return true;
		}
	}
}