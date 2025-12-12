#pragma once
#include "vm_types.hpp"

namespace MSLVM 
{

	void clear_vm_state(VMState& state);

	void execute_code_switch(VMState& state, VMOperation* operations, size_t length);

	void execute_code_compute_goto(VMState& state, VMOperation* operations, size_t length);

	//handlers

	void type_conversions(VMState& state, VMOperation& operation);
	void vm_calls(VMState& state, VMOperation& operation);
}