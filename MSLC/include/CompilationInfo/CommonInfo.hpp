#pragma once
#include "..\Definitions\ValueContainer.hpp"
#include "..\Definitions\CommonEnums.hpp"
#include <unordered_map>


namespace MSLC
{
	namespace CompilationInfo
	{

		using DescriptionID = size_t;
		constexpr DescriptionID INVALID_ID = SIZE_MAX;

		constexpr DescriptionID START_COUNTER_ID = 0;

		namespace Functions
		{
			using FunctionID = DescriptionID;
		}
		namespace Types
		{
			using TypeID = DescriptionID;
		}

		namespace Variables
		{

			using VariableID = DescriptionID;
		}


		namespace Values 
		{
			enum ValueFlags
			{
				None = 0,
				ConstValue = 1 << 0,
				ConstPointer = 1 << 1,
				Immediate = 1 << 2,		//100,true/false and another
				Pointer = 1 << 3,
				Link = 1 << 4,
			};

			using ConstantID = size_t;

			struct ValueInfo
			{
				Types::TypeID type_id = INVALID_ID;
				ConstantID immediate_const_id = INVALID_ID;	//in the constants table
				ValueFlags flags;
				uint16_t pointers_depth = 0;

				inline bool isPointer() const { return flags & Pointer; }
				inline bool isLink() const { return flags & Link; }
				inline bool isConstPointer() const { return flags & ConstPointer; }
				inline bool isConstValue() const { return flags & ConstValue; }
				inline bool isImmediate() const { return flags & Immediate; }
			};


			struct ImmediateConstantsTable
			{
				std::vector<Definitions::ValueContainer> constants_by_id;

				// Fast lookup: value → ID  
				std::unordered_map<Definitions::ValueContainer, ConstantID,
					Definitions::VCHash, Definitions::VCEqual> value_to_id;

				// Methods
				ConstantID GetOrAdd(const Definitions::ValueContainer& value) {
					auto it = value_to_id.find(value);
					if (it != value_to_id.end()) {
						return it->second;
					}

					ConstantID new_id = constants_by_id.size();
					constants_by_id.push_back(value);
					value_to_id[value] = new_id;
					return new_id;
				}

				const Definitions::ValueContainer& GetByID(ConstantID id) const {
					return constants_by_id[id];
				}

				bool Contains(const Definitions::ValueContainer& value) const {
					return value_to_id.find(value) != value_to_id.end();
				}

				ConstantID TryGetID(const Definitions::ValueContainer& value) const {
					auto it = value_to_id.find(value);
					if (it != value_to_id.end()) {
						return it->second;
					}
					return INVALID_ID;
				}
				void Clear() {
					constants_by_id.clear();
					value_to_id.clear();
				}
			};
		}

		
	}
}