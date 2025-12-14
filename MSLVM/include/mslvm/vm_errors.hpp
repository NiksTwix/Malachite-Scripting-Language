#pragma once
#include "vm_definitions.hpp"
#include "vm_stack.hpp"
#include "fwd.hpp"


namespace MSLVM 
{
	enum ErrorCode: uint8_t
	{
		NoError = 0,

		//Memory errors
		FailedMemoryFreeing,
		FailedMemoryAllocation,
		StackOverflow,
		StackUnderflow,
		InvalidMemoryAccess,

		FrameExpansionFailed,


		//Arithmetic
		ZeroDivision,
		NanValue,

		//VM Calls
		InvalidVMCall,
	};

	struct ErrorFrame 
	{
		size_t instruction_counter;
		ErrorCode code;
		//something else
	};

}