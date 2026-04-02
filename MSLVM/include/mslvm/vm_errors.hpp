#pragma once
#include "vm_definitions.hpp"
#include "vm_stack.hpp"
#include "fwd.hpp"


namespace MSLVM 
{
	enum ErrorCode : uint16_t
	{
		NoError = 0,

		SECTION_SCRIPT_LOADING = 1,	//Often are critical (doesn't allow start work)
		InvalidMagic,
		InvalidFilePath,
		InvalidFile,
		AnotherVMVersion,

		SECTION_OPERATIONS = 100,
		ZeroDivision,			//Critical
		NanValue,			//Major // Also inserts in register-destination nan value
		InfValue,			//Minor // Also inserts in register-destination nan value
		InvalidVMCall,			//Critical

		SECTION_MEMORY = 400,
		InvalidVMMemory,		//Critical		
		FailedMemoryFreeing,		//Major
		FailedMemoryAllocation,		//Major

		StackOverflow,			//Critical
		StackUnderflow,			//Critical

		InvalidMemoryAccess,		//Critical

		AttemptOfRODRewriting,		//Critical

		FrameExpansionFailed,		//Critical

		SECTION_UNNAMED = 800,		//For expansion
	};

	enum ErrorType : uint8_t
	{
		CRITICAL,	//Stops vm
		MAJOR,		//Doesnt stop vm
		MINOR,		//
		NO_ERROR,
	};

	struct ErrorFrame
	{
		size_t instruction_counter;
		uint64_t frame_fp = 0;
		uint64_t frame_sp = 0;
		ErrorCode code;
		ErrorType type;
	};
	
	struct ErrorsMap 
	{
	private:
		std::unordered_map<ErrorCode, ErrorType> errors =
		{
			{NoError,ErrorType::NO_ERROR},
			{InvalidMagic,ErrorType::CRITICAL},
			{InvalidFilePath,ErrorType::CRITICAL},
			{InvalidFile,ErrorType::CRITICAL},
			{AnotherVMVersion,ErrorType::CRITICAL},
			{ZeroDivision,	ErrorType::CRITICAL},
			{NanValue,	ErrorType::MAJOR},
			{InfValue,	ErrorType::MINOR},
			{InvalidVMCall,	ErrorType::CRITICAL},

			{InvalidVMMemory,ErrorType::CRITICAL},
			{FailedMemoryFreeing,ErrorType::MAJOR},
			{FailedMemoryAllocation,ErrorType::MAJOR},

			{StackOverflow,ErrorType::CRITICAL},
			{StackUnderflow,ErrorType::CRITICAL},

			{InvalidMemoryAccess,ErrorType::CRITICAL},

			{AttemptOfRODRewriting,ErrorType::CRITICAL},

			{FrameExpansionFailed,ErrorType::CRITICAL},
		};
	public:

		ErrorType GetType(ErrorCode code)
		{
			auto it = errors.find(code);
			if (it != errors.end()) return it->second;
			return ErrorType::NO_ERROR;
		}

		static ErrorsMap& Get() 
		{
			static ErrorsMap map;
			return map;
		}

	};

}