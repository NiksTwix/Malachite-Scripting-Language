#pragma once
#include <cstdint>
#include "vm_definitions.hpp"
#include "vm_linked_list.hpp"
#include <iostream>

#define BYTE 8

namespace MSLVM
{
    inline void write_little_endian(uint8_t* memory, uint64_t address, uint64_t value, size_t size) {
        for (size_t i = 0; i < size; i++) {
            memory[address+i] = static_cast<uint8_t>((value >> (i * BYTE)) & 0xFF);
        }
    }
    inline uint64_t read_little_endian(const uint8_t* memory, uint64_t address, size_t size) {
        uint64_t value = 0;
        for (size_t i = 0; i < size; i++) {
            value |= static_cast<uint64_t>(memory[address + i]) << (i * BYTE);
        }
        return value;
    }
    struct HeapFreeInterval // mode [...] - all inclusive
    {
        uint64_t start;
        uint64_t end;
        uint64_t size;

        HeapFreeInterval(uint64_t start, uint64_t end)
            : start(start), end(end), size(end - start + 1) {
        }

        // For debug
        bool contains(uint64_t addr) const {
            return addr >= start && addr <= end;
        }

        // Can we allocate size bytes?
        bool can_allocate(uint64_t alloc_size) const {
            return size >= alloc_size;
        }

        // Allocate from interval's start
        HeapFreeInterval allocate_from_start(uint64_t alloc_size) const {
            return HeapFreeInterval(start, start + alloc_size - 1);
        }

        // Another part after allocation
        HeapFreeInterval remaining_after_allocation(uint64_t alloc_size) const {
            return HeapFreeInterval(start + alloc_size, end);
        }
    };

    struct HeapFreeIntervals
    {
        LinkedList<HeapFreeInterval> free_intervals;

        HeapFreeIntervals(uint64_t heap_start, uint64_t heap_end)
        {
            free_intervals.push_back({ heap_start, heap_end });
        }

        // For debug
        void print() const {
            auto node = free_intervals.get_front();
            std::cout << "Free intervals:\n";
            while (node) {
                std::cout << "  [" << node->data.start << " - "
                    << node->data.end << "] size=" << node->data.size << "\n";
                node = node->next;
            }
        }
    };
    // Coalescing neightbor intervals
    inline void coalesce_free_intervals(HeapFreeIntervals& hfi)
    {
        auto current = hfi.free_intervals.get_front();
        while (current && current->next)
        {
            // If the current interval touches the next one 
            if (current->data.end + 1 == current->next->data.start)
            {
                // Merge
                current->data.end = current->next->data.end;
                current->data.size = current->data.end - current->data.start + 1;

                // Delete next interval
                hfi.free_intervals.remove(current->next);
                // Dont move current - check again
            }
            else
            {
                current = current->next;
            }
        }
    }
    inline bool allocate_memory(HeapFreeIntervals& hfi, uint64_t& address, size_t size)
    {
        // Start search from start
        hfi.free_intervals.reset_pointer();

        while (true)
        {
            auto current = hfi.free_intervals.get_current();
            if (!current) break;  // end of list

            if (current->data.size >= size)
            {
                address = current->data.start;

                if (current->data.size == size)
                {
                    //Delete all interval
                    hfi.free_intervals.remove(current);
                }
                else
                {
                    // Cut interval's start
                    current->data.start += size;
                    current->data.size = current->data.end - current->data.start + 1;
                }
                return true;
            }

            // Go to next interval
            if (!hfi.free_intervals.move_forward()) break;
        }

        return false;
    }

    //  Free memory (we must merge neighbor intervals)
    inline bool free_memory(HeapFreeIntervals& hfi, uint64_t address, size_t size)
    {
        uint64_t end = address + size - 1;
        auto new_interval = std::make_shared<typename LinkedList<HeapFreeInterval>::LinkedNode>(
            HeapFreeInterval(address, end));

        // Insert to sorted orded by start
        if (hfi.free_intervals.empty()) {
            hfi.free_intervals.push_back(new_interval->data);
        }
        else {
            // Search place for inserting
            auto node = hfi.free_intervals.get_front();
            std::shared_ptr<typename LinkedList<HeapFreeInterval>::LinkedNode> prev = nullptr;

            while (node && node->data.start < address) {
                prev = node;
                node = node->next;
            }

            if (prev) {
                // Insert after prev
                hfi.free_intervals.insert_after(prev, new_interval->data);
            }
            else {
                // Insert to start
                hfi.free_intervals.push_front(new_interval->data);
            }
        }

        // Coalescing neightbor intervals
        coalesce_free_intervals(hfi);
        return true;
    }
}