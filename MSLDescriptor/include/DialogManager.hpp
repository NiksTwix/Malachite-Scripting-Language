#pragma once
#include "Descriptor.hpp"
#include "MSLVM1Definitions.hpp"

class DialogManager 
{
private:
	Descriptor desc;

	enum class RODViewType: uint8_t
	{
		HEX_BYTES,
		DEC_BYTES,

		HEX_WORD,
		DEC_WORD,

		HEX_DWORD,
		DEC_DWORD,

		HEX_QWORD,
		DEC_QWORD,
	
	};

	std::unordered_map<std::string, RODViewType> rv_types = 
	{
		{"HEX_BYTES",RODViewType::HEX_BYTES},
		{"DEC_BYTES",RODViewType::DEC_BYTES},
		{"HEX_WORD",RODViewType::HEX_WORD},
		{"DEC_WORD",RODViewType::DEC_WORD},
		{"HEX_DWORD",RODViewType::HEX_DWORD},
		{"DEC_DWORD",RODViewType::DEC_DWORD},
		{"HEX_QWORD",RODViewType::HEX_QWORD},
		{"DEC_QWORD",RODViewType::DEC_QWORD},
	};
	std::unordered_map<RODViewType, std::string> rv_types_string =
	{
		{RODViewType::HEX_BYTES,"HEX_BYTES"},
		{RODViewType::DEC_BYTES,"DEC_BYTES"},
		{RODViewType::HEX_WORD,"HEX_WORD" },
		{RODViewType::DEC_WORD,"DEC_WORD" },
		{RODViewType::HEX_DWORD,"HEX_DWORD"},
		{RODViewType::DEC_DWORD,"DEC_DWORD"},
		{RODViewType::HEX_QWORD,"HEX_QWORD"},
		{RODViewType::DEC_QWORD,"DEC_QWORD"},
	};
	RODViewType rv_type = RODViewType::HEX_BYTES;

	void DialogROD(InterpretationState& state);
	void PrintDump(InterpretationState& state);
	void PrintCode(InterpretationState& state);
public:


	void DialogI();

	void Dialog();
};