#include "..\..\include\mslvm\vm_execution.hpp"
#include <cstring>


namespace MSLVM
{
	void clear_vm_state(VMState& state, bool clear_memory)
	{
		state.call_stack.clear();
		state.error_stack.clear();
		for (size_t i = 0; i < TOTAL_REGISTERS; i++)	//IP also will be zero
		{
			state.registers[i].u = 0;
		}
		state.registers[(uint64_t)SpecialRegister::SP].u = STACK_START;
		state.registers[(uint64_t)SpecialRegister::FP].u = STACK_START;

		if (clear_memory) 
		{
			memset(state.memory.memory, 0, STACK_SIZE + HEAP_SIZE);
		}
	}
	void execute_code_switch(VMState& state, VMOperation* operations, size_t length)
	{
		while (state.registers[SpecialRegister::IP].u < length)
		{
			VMOperation operation = *(operations + state.registers[SpecialRegister::IP].u);
			ErrorCode errcode{};

			switch (operation.code)
			{
				//--------------------Arithmetic Operations
				{
			case VMOperationCode::ADD_RRR_REAL:
				state.registers[REG_U(operation.arg0)].r = state.registers[REG_U(operation.arg1)].r + state.registers[REG_U(operation.arg2)].r; break;
			case VMOperationCode::ADD_RRR_UNSIGNED:
				state.registers[REG_U(operation.arg0)].u = state.registers[REG_U(operation.arg1)].u + state.registers[REG_U(operation.arg2)].u; break;
			case VMOperationCode::ADD_RRR_INTEGER:
				state.registers[REG_U(operation.arg0)].i = state.registers[REG_U(operation.arg1)].i + state.registers[REG_U(operation.arg2)].i; break;

			case VMOperationCode::SUB_RRR_REAL:
				state.registers[REG_U(operation.arg0)].r = state.registers[REG_U(operation.arg1)].r - state.registers[REG_U(operation.arg2)].r; break;
			case VMOperationCode::SUB_RRR_UNSIGNED:
				state.registers[REG_U(operation.arg0)].u = state.registers[REG_U(operation.arg1)].u - state.registers[REG_U(operation.arg2)].u; break;
			case VMOperationCode::SUB_RRR_INTEGER:
				state.registers[REG_U(operation.arg0)].i = state.registers[REG_U(operation.arg1)].i - state.registers[REG_U(operation.arg2)].i; break;

			case VMOperationCode::MUL_RRR_REAL:
				state.registers[REG_U(operation.arg0)].r = state.registers[REG_U(operation.arg1)].r * state.registers[REG_U(operation.arg2)].r; break;
			case VMOperationCode::MUL_RRR_UNSIGNED:
				state.registers[REG_U(operation.arg0)].u = state.registers[REG_U(operation.arg1)].u * state.registers[REG_U(operation.arg2)].u; break;
			case VMOperationCode::MUL_RRR_INTEGER:
				state.registers[REG_U(operation.arg0)].i = state.registers[REG_U(operation.arg1)].i * state.registers[REG_U(operation.arg2)].i; break;

			case VMOperationCode::DIV_RRR_REAL:
			{
				if (state.registers[REG_U(operation.arg2)].r == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[REG_U(operation.arg0)].r = state.registers[REG_U(operation.arg1)].r / state.registers[REG_U(operation.arg2)].r; break;
			}
			case VMOperationCode::DIV_RRR_UNSIGNED:
			{
				if (state.registers[REG_U(operation.arg2)].u == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[REG_U(operation.arg0)].u = state.registers[REG_U(operation.arg1)].u / state.registers[REG_U(operation.arg2)].u; break;
			}
			case VMOperationCode::DIV_RRR_INTEGER: {
				if (state.registers[REG_U(operation.arg2)].i == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[REG_U(operation.arg0)].i = state.registers[REG_U(operation.arg1)].i / state.registers[REG_U(operation.arg2)].i; break;
			}

			case VMOperationCode::NEG_RR_REAL:
				state.registers[REG_U(operation.arg0)].r = -state.registers[REG_U(operation.arg1)].r; break;
			case VMOperationCode::NEG_RR_INTEGER:
				state.registers[REG_U(operation.arg0)].i = -state.registers[REG_U(operation.arg1)].i; break;

			case VMOperationCode::MOD_RRR_UNSIGNED:
				if (state.registers[REG_U(operation.arg2)].u == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[REG_U(operation.arg0)].u = state.registers[REG_U(operation.arg1)].u % state.registers[REG_U(operation.arg2)].u; break;
			case VMOperationCode::MOD_RRR_INTEGER:
				if (state.registers[REG_U(operation.arg2)].i == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[REG_U(operation.arg0)].i = state.registers[REG_U(operation.arg1)].i % state.registers[REG_U(operation.arg2)].i; break;
				}
				//--------------------Logical Operations
				{
			case VMOperationCode::AND_RRR:
				state.registers[REG_U(operation.arg0)].u = (state.registers[REG_U(operation.arg1)].u != 0 && state.registers[REG_U(operation.arg2)].u != 0) ? 1 : 0;
				break;

			case VMOperationCode::OR_RRR:
				state.registers[REG_U(operation.arg0)].u = (state.registers[REG_U(operation.arg1)].u != 0 || state.registers[REG_U(operation.arg2)].u != 0) ? 1 : 0;
				break;

			case VMOperationCode::NOT_RR:
				state.registers[REG_U(operation.arg0)].u = (state.registers[REG_U(operation.arg1)].u == 0) ? 1 : 0;
				break;
				}

				//--------------------Bitwise Operations  
				{
					// Битовые операции
			case VMOperationCode::BIT_AND_RRR:
				state.registers[REG_U(operation.arg0)].u = state.registers[REG_U(operation.arg1)].u & state.registers[REG_U(operation.arg2)].u;
				break;

			case VMOperationCode::BIT_OR_RRR:
				state.registers[REG_U(operation.arg0)].u = state.registers[REG_U(operation.arg1)].u | state.registers[REG_U(operation.arg2)].u;
				break;

			case VMOperationCode::BIT_NOT_RR:
				state.registers[REG_U(operation.arg0)].u = ~state.registers[REG_U(operation.arg1)].u;
				break;

			case VMOperationCode::BIT_OFFSET_LEFT_RRR:
				state.registers[REG_U(operation.arg0)].u = state.registers[REG_U(operation.arg1)].u << state.registers[REG_U(operation.arg2)].u;
				break;

			case VMOperationCode::BIT_OFFSET_RIGHT_RRR:
				state.registers[REG_U(operation.arg0)].u = state.registers[REG_U(operation.arg1)].u >> state.registers[REG_U(operation.arg2)].u;
				break;
				}

				//--------------------Comparing Operations

				{
			case VMOperationCode::CMP_RR_INTEGER:
				//Reset logic flags state
				state.registers[SpecialRegister::FL].u &= ~(Flag::ZERO | Flag::LESS | Flag::GREATER);

				if (state.registers[REG_U(operation.arg0)].i == state.registers[REG_U(operation.arg1)].i) {
					state.registers[SpecialRegister::FL].u |= Flag::ZERO;
				}
				else if (state.registers[REG_U(operation.arg0)].i > state.registers[REG_U(operation.arg1)].i)
				{
					state.registers[SpecialRegister::FL].u |= Flag::GREATER;
				}
				else // Less case
				{
					state.registers[SpecialRegister::FL].u |= Flag::LESS;
				}
				break;
			case VMOperationCode::CMP_RR_UNSIGNED:
				//Reset logic flags state
				state.registers[SpecialRegister::FL].u &= ~(Flag::ZERO | Flag::LESS | Flag::GREATER);
				if (state.registers[REG_U(operation.arg0)].u == state.registers[REG_U(operation.arg1)].u) {
					state.registers[SpecialRegister::FL].u |= Flag::ZERO;
				}
				else if (state.registers[REG_U(operation.arg0)].u > state.registers[REG_U(operation.arg1)].u)
				{
					state.registers[SpecialRegister::FL].u |= Flag::GREATER;
				}
				else // Less case
				{
					state.registers[SpecialRegister::FL].u |= Flag::LESS;
				}
				break;
			case VMOperationCode::CMP_RR_REAL:
			{
				//Reset logic flags state
				state.registers[SpecialRegister::FL].u &= ~(Flag::ZERO | Flag::LESS | Flag::GREATER);

				auto is_nan = [](double value) -> bool {
					uint64_t bits = *reinterpret_cast<const uint64_t*>(&value);
					uint64_t exponent = (bits >> 52) & 0x7FF;  // 11 bits for exponent
					uint64_t mantissa = bits & 0xFFFFFFFFFFFFF; // 52 bits for mantissa
					return (exponent == 0x7FF) && (mantissa != 0);
					};
				if (is_nan(state.registers[REG_U(operation.arg0)].r) || is_nan(state.registers[REG_U(operation.arg1)].r)) {
					// NaN detected - don't set comparison flags
					errcode = ErrorCode::NanValue; break;
				}
				if (state.registers[REG_U(operation.arg0)].r == state.registers[REG_U(operation.arg1)].r) {
					state.registers[SpecialRegister::FL].u |= Flag::ZERO;
				}
				else if (state.registers[REG_U(operation.arg0)].r > state.registers[REG_U(operation.arg1)].r)
				{
					state.registers[SpecialRegister::FL].u |= Flag::GREATER;
				}
				else // Less case
				{
					state.registers[SpecialRegister::FL].u |= Flag::LESS;
				}
				break;
			}
			break;
				}
				//--------------------Memory Operations                                      
				{
			case MOV_RR:
				state.registers[REG_U(operation.arg0)] = state.registers[REG_U(operation.arg1)];
				break;
			case MOV_RI:
				state.registers[REG_U(operation.arg0)] = operation.arg1;
				break;
			case PUSH:
			{
				uint64_t size = REG_U(operation.arg1);
				uint64_t current_sp = state.registers[SpecialRegister::SP].u;
				uint64_t new_sp = current_sp + size;

				if (new_sp > STACK_END) {
					errcode = ErrorCode::StackOverflow;
					break;
				}

				uint64_t value = state.registers[REG_U(operation.arg0)].u;
				write_little_endian(state.memory.memory, current_sp, value, size);

				state.registers[SpecialRegister::SP].u = new_sp;
				break;
			}

			case POP:
			{
				uint64_t size = operation.arg1.u;
				uint64_t current_sp = state.registers[SpecialRegister::SP].u;

				if (current_sp < size) {
					errcode = ErrorCode::StackUnderflow;
					break;
				}

				uint64_t new_sp = current_sp - size;

				uint64_t value = read_little_endian(state.memory.memory, new_sp, size);
				state.registers[REG_U(operation.arg0)].u = value;

				// Обновление SP
				state.registers[SpecialRegister::SP].u = new_sp;
				break;
			}
			case LOAD_LOCAL:
			{
				uint64_t offset = operation.arg1.u;
				uint64_t size = operation.arg2.u;

				// Local variable are AFTER FP (stack grows to top)
				uint64_t address = state.registers[SpecialRegister::FP].u + offset;

				// Checking
				if (address > STACK_END || size > STACK_END - address + 1) {
					errcode = ErrorCode::InvalidMemoryAccess;
					break;
				}

				// Read little-endian
				uint64_t value = read_little_endian(state.memory.memory, address, size);
				state.registers[REG_U(operation.arg0)].u = value;
				break;
			}

			case STORE_LOCAL: 
			{
				uint64_t offset = REG_U(operation.arg1);
				uint64_t size = operation.arg2.u;
				uint64_t value = state.registers[REG_U(operation.arg0)].u;

				uint64_t address = state.registers[SpecialRegister::FP].u + offset;

				// Проверка границ
				if (address > STACK_END || size > STACK_END - address + 1) {
					errcode = ErrorCode::InvalidMemoryAccess;
					break;
				}

				// Write little-endian
				write_little_endian(state.memory.memory, address, value, size);
				break;
			}
			case LOAD_BY_ADDRESS: 
			{
				uint64_t address = state.registers[REG_U(operation.arg1)].u;
				uint64_t size = operation.arg2.u;

				// Checking
				if (size > HEAP_END - address + 1) {
					errcode = ErrorCode::InvalidMemoryAccess;
					break;
				}

				// Read little-endian
				uint64_t value = read_little_endian(state.memory.memory, address, size);
				state.registers[REG_U(operation.arg0)].u = value;
				break;
			}

			case STORE_BY_ADDRESS: 
			{
				uint64_t value = state.registers[REG_U(operation.arg0)].u;
				uint64_t address = state.registers[REG_U(operation.arg1)].u;
				uint64_t size = operation.arg2.u;


				// Checking
				if (size > HEAP_END - address + 1) {
					errcode = ErrorCode::InvalidMemoryAccess;
					break;
				}

				// Write little-endian
				write_little_endian(state.memory.memory, address, value, size);
				break;
			}
		
			case CALC_FRAME_ADDRESS:        
			{
				uint64_t offset = operation.arg1.u;

				state.registers[REG_U(operation.arg0)].u = state.registers[SpecialRegister::FP].u + offset;

				break;
			}

			case ALLOCATE_MEMORY:
			{
				uint64_t address = HEAP_START;
				if (!allocate_memory(state.memory.HFI, address, state.registers[REG_U(operation.arg1)].u))
				{
					errcode = ErrorCode::FailedMemoryAllocation;
					break;
				}
				state.registers[REG_U(operation.arg0)].u =  address;
				break;
			}
			case FREE_MEMORY:
			{
				uint64_t address = state.registers[REG_U(operation.arg0)].u;
				uint64_t size = state.registers[REG_U(operation.arg1)].u;
				if (address < HEAP_START ||  size > HEAP_END - address + 1) {
					errcode = ErrorCode::InvalidMemoryAccess;
					break;
				}
				if (!free_memory(state.memory.HFI, address, size)) {
					errcode = ErrorCode::FailedMemoryFreeing;
					break;
				}
				break;
			}

			case GRAB_FRAME:
			{
				uint64_t expanded_bytes = REG_U(operation.arg0);

				if (state.call_stack.empty()) {
					errcode = ErrorCode::FrameExpansionFailed;
					break;
				}

				auto& t = state.call_stack.top();
				uint64_t new_fp = state.registers[SpecialRegister::FP].u - expanded_bytes;

				if (new_fp < STACK_START || new_fp > STACK_END) {
					errcode = ErrorCode::StackOverflow;
					break;
				}

				if (new_fp < t.fp) {  
					errcode = ErrorCode::FrameExpansionFailed;
					break;
				}

				t.sp -= expanded_bytes;
				state.registers[SpecialRegister::FP].u = new_fp;
				break;
			}
				}

				//--------------------Control Flow Operations  
				{
			case VMOperationCode::JMP:
				state.registers[SpecialRegister::IP].u = REG_U(operation.arg0);
				state.registers[SpecialRegister::FL].u |= Flag::JUMPED;
				break;
			case VMOperationCode::JMP_CV:
				if (state.registers[REG_U(operation.arg1)].u != 0)
				{
					state.registers[SpecialRegister::IP].u = REG_U(operation.arg0);
					state.registers[SpecialRegister::FL].u |= Flag::JUMPED;
				}
				break;
			case VMOperationCode::JMP_CNV:
				if (state.registers[REG_U(operation.arg1)].u == 0)
				{
					state.registers[SpecialRegister::IP].u = REG_U(operation.arg0);
					state.registers[SpecialRegister::FL].u |= Flag::JUMPED;
				}
				break;
			case VMOperationCode::CALL:
				if (state.call_stack.size() >= CALL_STACK_SIZE)
				{
					errcode = ErrorCode::StackOverflow;
					break;
				}
				//Save current state
				state.call_stack.push(CallFrame(state.registers[SpecialRegister::SP].u,
					state.registers[SpecialRegister::FP].u,
					state.registers[SpecialRegister::IP].u));

				state.registers[SpecialRegister::FP].u = state.registers[SpecialRegister::SP].u;

				state.registers[SpecialRegister::IP].u = REG_U(operation.arg0);
				state.registers[SpecialRegister::FL].u |= Flag::JUMPED;

				break;
			case VMOperationCode::RET:
			{
				if (state.call_stack.empty()) {
					errcode = ErrorCode::StackUnderflow;
					break;
				}
				CallFrame cf = state.call_stack.pop();

				state.registers[SpecialRegister::SP].u = cf.sp;
				state.registers[SpecialRegister::FP].u = cf.fp;

				state.registers[SpecialRegister::IP].u = cf.ip;
				//JUMPED flag doesnt be setted because we need to skip call instruction
				break;
			}
			
			case VMOperationCode::HALT:
				state.registers[SpecialRegister::FL].u |= Flag::STOPPED;
				break;
				}

				//--------------------VM Calls Operations  
				{
			case VMOperationCode::VM_CALL:
					handle_vm_call(state, operation);
					break;
				}

				//--------------------Type Convertion Operations  
				{
			case VMOperationCode::TC_ITR_R: 
				state.registers[REG_U(operation.arg0)].r = static_cast<register_real>(state.registers[REG_U(operation.arg0)].i);
				break;
			case VMOperationCode::TC_RTI_R: 
				state.registers[REG_U(operation.arg0)].i = static_cast<register_integer>(state.registers[REG_U(operation.arg0)].r);
				break;
			case VMOperationCode::TC_UTR_R: 
				state.registers[REG_U(operation.arg0)].r = static_cast<register_real>(state.registers[REG_U(operation.arg0)].u);
				break;
			case VMOperationCode::TC_UTI_R: 
				state.registers[REG_U(operation.arg0)].i = static_cast<register_integer>(state.registers[REG_U(operation.arg0)].u);
				break;
			case VMOperationCode::TC_RTU_R: 
				state.registers[REG_U(operation.arg0)].u = static_cast<register_unsigned>(state.registers[REG_U(operation.arg0)].r);
				break;
			case VMOperationCode::TC_ITU_R: 
				state.registers[REG_U(operation.arg0)].u = static_cast<register_unsigned>(state.registers[REG_U(operation.arg0)].i);
				break;

				}
			default:
				break;
			}

			if (errcode != ErrorCode::NoError)
			{
				ErrorFrame ef;
				ef.code = errcode;
				ef.instruction_counter = state.registers[SpecialRegister::IP].u;
				state.error_stack.push(ef);
				state.registers[SpecialRegister::FL].u |= (uint64_t)Flag::STOPPED;
				break;
			}
			if (state.registers[SpecialRegister::FL].u & JUMPED) continue;
			if (state.registers[SpecialRegister::FL].u & STOPPED) break;	//flags reset in "clear_vm_state" function

			state.registers[SpecialRegister::IP].u += 1;
		}
	}
#if defined(CLANG_OR_GNUC)
	void execute_code_compute_goto(VMState& state, VMOperation* operations, size_t length)
	{
	}
#endif
	void handle_vm_call(VMState& state, VMOperation& operation)
	{
		auto call = REG_U(operation.arg0);
		switch (call)
		{
			case VMCallType::PRINT_INTEGER:
			{
				int64_t value = state.registers[REG_U(operation.arg1)].i;	//Take value from register
				std::cout << value;
				break;
			}
			case VMCallType::PRINT_UNSIGNED:
			{
				uint64_t value = state.registers[REG_U(operation.arg1)].u;	//Take value from register
				std::cout << value;
				break;
			}
			case  VMCallType::PRINT_CHAR:
			{
				putchar(state.registers[REG_U(operation.arg1)].i);//Take value from register
				break;
			}
			case  VMCallType::PRINT_REAL:
			{
				double value = state.registers[REG_U(operation.arg1)].r;	//Take value from register
				std::cout << value;
				break;
			}
			default: break;
		}
	}
}

