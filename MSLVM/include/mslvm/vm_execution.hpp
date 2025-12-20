#pragma once
#include "vm_types.hpp"

#if defined(__clang__) || defined(__GNUC__)
#define CLANG_OR_GNUC 1
#endif

namespace MSLVM 
{

	void clear_vm_state(VMState& state, bool clear_memory);

	void execute_code_switch(VMState& state, VMOperation* operations, size_t length);

#if defined(CLANG_OR_GNUC)
	void execute_code_compute_goto(VMState& state, VMOperation* operations, size_t length);
#endif


	void handle_vm_call(VMState& state, VMOperation& operation);

}