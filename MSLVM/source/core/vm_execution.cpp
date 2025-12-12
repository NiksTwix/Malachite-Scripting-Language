#include "..\..\include\mslvm\vm_execution.hpp"
#include "..\..\include\mslvm\vm_memory.hpp"


namespace MSLVM
{
	void execute_code_switch(VMState& state, VMOperation* operations, size_t length)
	{
		state.registers[SpecialRegister::IP].u = 0;

		while (state.registers[SpecialRegister::IP].u < length)
		{
			VMOperation operation = *(operations + state.registers[SpecialRegister::IP].u);
			ErrorCode errcode{};

			switch (operation.code)
			{
				//--------------------Arithmetic Operations
				{
			case VMOperationCode::ADD_RRR_REAL:
				state.registers[operation.dest].r = state.registers[operation.src0].r + state.registers[operation.src1].r; break;
			case VMOperationCode::ADD_RRR_UNSIGNED:
				state.registers[operation.dest].u = state.registers[operation.src0].u + state.registers[operation.src1].u; break;
			case VMOperationCode::ADD_RRR_INTEGER:
				state.registers[operation.dest].i = state.registers[operation.src0].i + state.registers[operation.src1].i; break;

			case VMOperationCode::SUB_RRR_REAL:
				state.registers[operation.dest].r = state.registers[operation.src0].r - state.registers[operation.src1].r; break;
			case VMOperationCode::SUB_RRR_UNSIGNED:
				state.registers[operation.dest].u = state.registers[operation.src0].u - state.registers[operation.src1].u; break;
			case VMOperationCode::SUB_RRR_INTEGER:
				state.registers[operation.dest].i = state.registers[operation.src0].i - state.registers[operation.src1].i; break;

			case VMOperationCode::MUL_RRR_REAL:
				state.registers[operation.dest].r = state.registers[operation.src0].r * state.registers[operation.src1].r; break;
			case VMOperationCode::MUL_RRR_UNSIGNED:
				state.registers[operation.dest].u = state.registers[operation.src0].u * state.registers[operation.src1].u; break;
			case VMOperationCode::MUL_RRR_INTEGER:
				state.registers[operation.dest].i = state.registers[operation.src0].i * state.registers[operation.src1].i; break;

			case VMOperationCode::DIV_RRR_REAL:
			{
				if (state.registers[operation.src1].r == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[operation.dest].r = state.registers[operation.src0].r / state.registers[operation.src1].r; break;
			}
			case VMOperationCode::DIV_RRR_UNSIGNED:
			{
				if (state.registers[operation.src1].u == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[operation.dest].u = state.registers[operation.src0].u / state.registers[operation.src1].u; break;
			}
			case VMOperationCode::DIV_RRR_INTEGER: {
				if (state.registers[operation.src1].i == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[operation.dest].i = state.registers[operation.src0].i / state.registers[operation.src1].i; break;
			}

			case VMOperationCode::NEG_RR_REAL:
				state.registers[operation.dest].r = -state.registers[operation.src0].r; break;
			case VMOperationCode::NEG_RR_INTEGER:
				state.registers[operation.dest].i = -state.registers[operation.src0].i; break;

			case VMOperationCode::MOD_RR_UNSIGNED:
				if (state.registers[operation.src1].u == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[operation.dest].u = state.registers[operation.src0].u % state.registers[operation.src1].u; break;
			case VMOperationCode::MOD_RR_INTEGER:
				if (state.registers[operation.src1].i == 0) {
					errcode = ErrorCode::ZeroDivision; break;
				}
				state.registers[operation.dest].i = state.registers[operation.src0].i % state.registers[operation.src1].i; break;
				}
				//--------------------Logical Operations
				{
			case VMOperationCode::AND_RRR:
				state.registers[operation.dest].u = (state.registers[operation.src0].u != 0 && state.registers[operation.src1].u != 0) ? 1 : 0;
				break;

			case VMOperationCode::OR_RRR:
				state.registers[operation.dest].u = (state.registers[operation.src0].u != 0 || state.registers[operation.src1].u != 0) ? 1 : 0;
				break;

			case VMOperationCode::NOT_RR:
				state.registers[operation.dest].u = (state.registers[operation.src0].u == 0) ? 1 : 0;
				break;
				}

				//--------------------Bitwise Operations  
				{
					// Битовые операции
			case VMOperationCode::BIT_AND_RRR:
				state.registers[operation.dest].u = state.registers[operation.src0].u & state.registers[operation.src1].u;
				break;

			case VMOperationCode::BIT_OR_RRR:
				state.registers[operation.dest].u = state.registers[operation.src0].u | state.registers[operation.src1].u;
				break;

			case VMOperationCode::BIT_NOT_RR:
				state.registers[operation.dest].u = ~state.registers[operation.src0].u;
				break;

			case VMOperationCode::BIT_OFFSET_LEFT_RRR:
				state.registers[operation.dest].u = state.registers[operation.src0].u << state.registers[operation.src1].u;
				break;

			case VMOperationCode::BIT_OFFSET_RIGHT_RRR:
				state.registers[operation.dest].u = state.registers[operation.src0].u >> state.registers[operation.src1].u;
				break;
				}

				//--------------------Comparing Operations

				{
			case VMOperationCode::CMP_RR_INTEGER:
				//Reset logic flags state
				state.registers[SpecialRegister::FL].u &= ~(Flag::ZERO | Flag::LESS | Flag::GREATER);

				if (state.registers[operation.src0].i == state.registers[operation.src1].i) {
					state.registers[SpecialRegister::FL].u |= Flag::ZERO;
				}
				else if (state.registers[operation.src0].i > state.registers[operation.src1].i)
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
				if (state.registers[operation.src0].u == state.registers[operation.src1].u) {
					state.registers[SpecialRegister::FL].u |= Flag::ZERO;
				}
				else if (state.registers[operation.src0].u > state.registers[operation.src1].u)
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
				if (is_nan(state.registers[operation.src0].r) || is_nan(state.registers[operation.src1].r)) {
					// NaN detected - don't set comparison flags
					errcode = ErrorCode::NanValue; break;
				}
				if (state.registers[operation.src0].r == state.registers[operation.src1].r) {
					state.registers[SpecialRegister::FL].u |= Flag::ZERO;
				}
				else if (state.registers[operation.src0].r > state.registers[operation.src1].r)
				{
					state.registers[SpecialRegister::FL].u |= Flag::GREATER;
				}
				else // Less case
				{
					state.registers[SpecialRegister::FL].u |= Flag::LESS;
				}
				break;
			}
			case VMOperationCode::GET_FLAG: //destination - register, source0 - flag type (check FLAG enum)
			{
				Flag flag = (Flag)operation.src0;	// uint64_t -> uint32_t
				state.registers[operation.dest].u = state.registers[SpecialRegister::FL].u & flag;
			}
			break;
				}
				//--------------------Memory Operations                                      
				{
			case MOV_RR:
				state.registers[operation.dest].u = state.registers[operation.src0].u;
				break;
			case MOV_RI_INTEGER:
				state.registers[operation.dest].i = operation.imm.i;
				break;
			case MOV_RI_UNSIGNED:
				state.registers[operation.dest].u = operation.imm.u;
				break;
			case MOV_RI_REAL:
				state.registers[operation.dest].r = operation.imm.r;
				break;
			case PUSH:
			{
				uint64_t size = operation.dest;

				if (state.registers[SpecialRegister::SP].u + size > STACK_END) {	//OVERFLOW OF UINT64_T
					errcode = ErrorCode::StackOverflow;
					break;
				}
				uint64_t start_address = state.registers[SpecialRegister::SP].u;	//register subtracted on size before
				uint64_t value = state.registers[operation.src0].u;
				write_little_endian(state.memory.stack, start_address, value, size);
				state.registers[SpecialRegister::SP].u += size;
				break;
			}

			case POP:
			{
				uint64_t size = operation.src0;
				uint64_t sp = state.registers[SpecialRegister::SP].u;

				//  Underflow checking: subtraction size from SP is fobidden if SP < size 
				if (sp < size) {  // sp - size приведёт к underflow через 0
					errcode = ErrorCode::StackUnderflow;
					break;
				}
				state.registers[SpecialRegister::SP].u -= size;
				uint64_t start_address = state.registers[SpecialRegister::SP].u;
				state.registers[operation.dest].u = read_little_endian(state.memory.stack, start_address, size);
				break;
			}
			case LOAD_LOCAL: // dest-register, src0-offset from FP, src1-data size (in bytes)
			{
				uint64_t offset = operation.src0;
				uint64_t size = operation.src1;

				// Local variable are AFTER FP (stack grows to top)
				uint64_t address = state.registers[SpecialRegister::FP].u + offset;

				// Checking
				if (address > STACK_END || address + size - 1 > STACK_END) {
					errcode = ErrorCode::InvalidMemoryAccess;
					break;
				}

				// Read little-endian
				uint64_t value = read_little_endian(state.memory.stack, address, size);
				state.registers[operation.dest].u = value;
				break;
			}

			case STORE_LOCAL: // dest-offset from FP, src0-register, src1-data size
			{
				uint64_t offset = operation.dest;
				uint64_t size = operation.src1;
				uint64_t value = state.registers[operation.src0].u;

				uint64_t address = state.registers[SpecialRegister::FP].u + offset;

				// Проверка границ
				if (address > STACK_END || address + size - 1 > STACK_END) {
					errcode = ErrorCode::InvalidMemoryAccess;
					break;
				}

				// Write little-endian
				write_little_endian(state.memory.stack, address, value, size);
				break;
			}

			case LOAD_GLOBAL: // dest-register, src0-offset from STACK_START, src1-data size
			{
				uint64_t offset = operation.src0;
				uint64_t size = operation.src1;

				// Global variables are in bottom part of stack  (near to STACK_START)
				// offset - is offset from STACK_START to TOP
				uint64_t address = STACK_START + offset;

				// Checking
				if (address > STACK_END || address + size - 1 > STACK_END) {
					errcode = ErrorCode::InvalidMemoryAccess;
					break;
				}

				// Read little-endian
				uint64_t value = read_little_endian(state.memory.stack, address, size);
				state.registers[operation.dest].u = value;
				break;
			}

			case STORE_GLOBAL: // dest-offset from STACK_START, src0-register, src1-data size
			{
				uint64_t offset = operation.dest;
				uint64_t size = operation.src1;
				uint64_t value = state.registers[operation.src0].u;

				uint64_t address = STACK_START + offset;

				// Checking
				if (address > STACK_END || address + size - 1 > STACK_END) {
					errcode = ErrorCode::InvalidMemoryAccess;
					break;
				}

				// Write little-endian
				write_little_endian(state.memory.stack, address, value, size);
				break;
			}
			case ALLOCATE_MEMORY: break;
			case FREE_MEMORY: break;
				}

				//--------------------Control Flow Operations  
				{
			case VMOperationCode::JMP:
				state.registers[SpecialRegister::IP].u = operation.dest;
				state.registers[SpecialRegister::FL].u |= Flag::JUMPED;
				break;
			case VMOperationCode::JMP_CV:
				if (state.registers[operation.src0].u != 0)
				{
					state.registers[SpecialRegister::IP].u = operation.dest;
					state.registers[SpecialRegister::FL].u |= Flag::JUMPED;
				}
				break;
			case VMOperationCode::JMP_CNV:
				if (state.registers[operation.src0].u == 0)
				{
					state.registers[SpecialRegister::IP].u = operation.dest;
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

				state.registers[SpecialRegister::IP].u = operation.dest;
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
}

