#pragma once
#include "vm_definitions.hpp"

namespace MSLVM
{
	struct VMState;
	struct CallFrame;
	struct VMOperation;

	struct LinearMemory;
	struct Register;

	template<typename T, size_t MAX_SIZE>
	class VMStack;

	using CallStack = VMStack<CallFrame, CALL_STACK_SIZE>;


	struct ErrorFrame;
	using ErrorStack = VMStack<ErrorFrame, ERROR_STACK_SIZE>;

	struct HeapFreeIntervals;

}