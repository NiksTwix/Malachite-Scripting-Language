#pragma once

#include "vm_linked_list.hpp"
#include "vm_errors.hpp"
#include <iostream>
#include <cstdint>


#define BYTE 8
#define VALID_MEMORY_ACCESS_SIZE 8

#define INTERVAL_SIZE(start, end) ((end) - (start) + 1)


namespace MSLVM
{
   

    struct alignas(8) DynamicMemory		//LittleEndian      Stack is static. Heap is dynamic(can be resized) 
    {
    private:
        uint8_t* m_pointer = nullptr;

        size_t code_sa = 0;		//sa - start address, ea - end address (last + 1)
        size_t rod_sa = 0;
        size_t stack_sa = 0;
        size_t heap_sa = 0;
        size_t size = 0;

        size_t end = 0;

        

        size_t code_size = 0;   //constant
        size_t rod_size = 0;    //constant
        size_t stack_size = 0;
        size_t heap_size = 0;
       

        inline size_t GetSize(size_t first, size_t second) { if (second < first) return 0; return second - first; }

        void TransferData(size_t new_stack_size, size_t new_heap_size);
     

        inline bool InInterval(size_t istart, size_t iend, size_t place)
        {
            return place >= istart && place <= iend;
        }


        ErrorCode status = ErrorCode::NoError;

    public:
        DynamicMemory() = default;

        void Allocate(size_t code_size, size_t rod_size);

        uint64_t Read(uint64_t address, size_t size);		//64 - max size of data/ Little Endian
        uint8_t ReadByte(uint64_t address);
        void Write(uint64_t address, uint64_t value, size_t size);		//64 - max size of data/ Little Endian

        VMOperation& GetOperation(size_t index);

        inline size_t GetOperationsCount() { return code_size / sizeof(VMOperation); }


        void Free();
        void IncreaseHeap();	//increase heap 2 times more
        void DencreaseHeap();	//IDK

        inline size_t GetStackStart() { return stack_sa; }
        inline size_t GetHeapStart() { return heap_sa; }
        inline size_t GetEnd() { return end; }
        inline size_t GetSize() { return size; }
        inline bool IsValid() { return m_pointer != nullptr && status == NoError; }

        inline bool CheckIntervals(size_t start, size_t end, size_t address, size_t size) //end = last_valid address + 1
        {
            if (address < start) return false;
            if (address >= end) return false;
            if (address + size > end) return false;
            return true;
        }

        inline size_t ToNative(size_t address) 
        {
            size_t result = code_size + address;
            return result;
        }

        void ClearDynamicPart();    //Stack + heap

        inline ErrorCode GetStatus() { return status; }

        bool LoadCode(char* code_bytes, size_t size);
        bool LoadROD(char* rod_bytes, size_t size);
    };


    struct HeapFreeInterval {
        uint64_t start;
        uint64_t end;
        uint64_t size;

        HeapFreeInterval(uint64_t start = 0, uint64_t end = 0)
            : start(start), end(end), size(INTERVAL_SIZE(start, end)) {
        }

        bool Contains(uint64_t addr) const {
            return addr >= start && addr <= end;
        }

        bool CanAllocate(uint64_t alloc_size) const {
            return size >= alloc_size;
        }
        // Split block: returns 1 if remainder exists , 0 if not
        // allocated - exit parameter for allocated part 
        // remaining - exit parameter for remainder (can be nullptr)
        int Split(uint64_t alloc_size, HeapFreeInterval* allocated, HeapFreeInterval* remaining) const {
            if (allocated == nullptr) return 0;

            *allocated = HeapFreeInterval(start, start + alloc_size - 1);

            if (alloc_size >= size) {
                // All block used
                if (remaining) {
                    *remaining = HeapFreeInterval();
                }
                return 0; // Remainder doesnt exist
            }
            else {
                // Remainder exists
                if (remaining) {
                    *remaining = HeapFreeInterval(start + alloc_size, end);
                }
                return 1; // Remainder exists
            }
        }

        // Can merge with another block? 
        bool CanMergeWith(const HeapFreeInterval& other) const {
            return (end + 1 == other.start) || (other.end + 1 == start);
        }

        // Merge with another block
        HeapFreeInterval MergeWith(const HeapFreeInterval& other) const {
            uint64_t new_start = start < other.start ? start : other.start;
            uint64_t new_end = end > other.end ? end : other.end;
            return HeapFreeInterval(new_start, new_end);
        }

        // For debug
        void Print() const {
            std::cout << "[" << start << " - " << end << "] size=" << size;
        }

        bool operator==(const HeapFreeInterval& other) const {
            return start == other.start && end == other.end;
        }
    };
    struct HeapFreeIntervals {
        // size ->  list of blocks of this size
        std::unordered_map<size_t, LinkedList<HeapFreeInterval>> free_intervals_by_size;

        // Also contain list with addresses for fast coalescing
        LinkedList<HeapFreeInterval> free_intervals_by_addr;

        // Minimum and maximum size for search optimization
        size_t min_block_size = SIZE_MAX;
        size_t max_block_size = 0;

        size_t alignment = DEFAULT_ALIGNMENT; // Default alignment is 8 byte
        HeapFreeIntervals(uint64_t heap_start, uint64_t heap_end, size_t align = DEFAULT_ALIGNMENT)
            : alignment(align) {
            // Init heap with one big block
            HeapFreeInterval initial(heap_start, heap_end);
            AddInterval(initial);


        }
        HeapFreeIntervals() = default;

        void Set(uint64_t heap_start, uint64_t heap_end, size_t align = DEFAULT_ALIGNMENT) 
        {
            alignment = align;
            HeapFreeInterval initial(heap_start, heap_end);
            AddInterval(initial);
        }

        // Append interval in both indices 
        void AddInterval(const HeapFreeInterval& interval);
        // Remove interval from indices
        void RemoveInterval(const HeapFreeInterval& interval);
        // Find necessary block for allocation
        // Returns pointer on node with block or nullptr
        std::shared_ptr<typename LinkedList<HeapFreeInterval>::LinkedNode> FindFitBlock(uint64_t required_size, HeapFreeInterval* out_block = nullptr);

        // Allocation with alignment
        // address - exit parameter
        ErrorCode Allocate(uint64_t size, uint64_t* address);
        // Allocation with specific alignment
        ErrorCode AllocateAligned(uint64_t size, size_t specific_alignment, uint64_t* address);
        // Memory freeing
        ErrorCode Free(uint64_t address, uint64_t size);

        // Merge adjacent blocks
        void Coalesce() {
            free_intervals_by_addr.coalesce(
                // Checking: can merge?
                [](const HeapFreeInterval& a, const HeapFreeInterval& b) {
                    return a.CanMergeWith(b);
                },
                // Merge function
                [](const HeapFreeInterval& a, const HeapFreeInterval& b) {
                    return a.MergeWith(b);
                }
            );

            // After coalescing we need update index by size
            RebuildSizeIndex();
        }

        // Rebuild indices by size after coalescing
        void RebuildSizeIndex();


    };


  
}