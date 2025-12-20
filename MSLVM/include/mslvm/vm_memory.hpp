#pragma once
#include <cstdint>
#include "vm_linked_list.hpp"
#include <iostream>



#define BYTE 8
#define VALID_MEMORY_ACCESS_SIZE 8

#define INVALID_ADDRESS UINT64_MAX
#define INTERVAL_SIZE(start, end) ((end) - (start) + 1)
#define ALLOC_SUCCESS 0
#define ALLOC_FAILURE 1
#define FREE_SUCCESS 0
#define FREE_FAILURE 1

namespace MSLVM
{

    inline void write_little_endian(uint8_t* memory, uint64_t address, uint64_t value, size_t size) {

        size = size > VALID_MEMORY_ACCESS_SIZE? VALID_MEMORY_ACCESS_SIZE : size;
        for (size_t i = 0; i < size; i++) {
            memory[address+i] = static_cast<uint8_t>((value >> (i * BYTE)) & 0xFF);
        }
    }
    inline uint64_t read_little_endian(const uint8_t* memory, uint64_t address, size_t size) {
        uint64_t value = 0;
        size = size > VALID_MEMORY_ACCESS_SIZE ? VALID_MEMORY_ACCESS_SIZE : size;
        for (size_t i = 0; i < size; i++) {
            value |= static_cast<uint64_t>(memory[address + i]) << (i * BYTE);
        }
        return value;
    }

    struct HeapFreeInterval {
        uint64_t start;
        uint64_t end;
        uint64_t size;

        HeapFreeInterval(uint64_t start = 0, uint64_t end = 0)
            : start(start), end(end), size(INTERVAL_SIZE(start, end)) {
        }

        bool contains(uint64_t addr) const {
            return addr >= start && addr <= end;
        }

        bool can_allocate(uint64_t alloc_size) const {
            return size >= alloc_size;
        }

        // Split block: returns 1 if remainder exists , 0 if not
        // allocated - exit parameter for allocated part 
        // remaining - exit parameter for remainder (can be nullptr)
        int split(uint64_t alloc_size, HeapFreeInterval* allocated, HeapFreeInterval* remaining) const {
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
        bool can_merge_with(const HeapFreeInterval& other) const {
            return (end + 1 == other.start) || (other.end + 1 == start);
        }

        // Merge with another block
        HeapFreeInterval merge_with(const HeapFreeInterval& other) const {
            uint64_t new_start = start < other.start ? start : other.start;
            uint64_t new_end = end > other.end ? end : other.end;
            return HeapFreeInterval(new_start, new_end);
        }

        // For debug
        void print() const {
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

        size_t alignment = 1; // Default alignment is 1 byte

        HeapFreeIntervals(uint64_t heap_start, uint64_t heap_end, size_t align = 1)
            : alignment(align) {
            // Init heap with one big block
            HeapFreeInterval initial(heap_start, heap_end);
            add_interval(initial);
        }

        // Append interval in both indices 
        void add_interval(const HeapFreeInterval& interval) {
            // Добавить в список по адресу (отсортированно)
            free_intervals_by_addr.insert_sorted(interval,
                [](const HeapFreeInterval& a, const HeapFreeInterval& b) {
                    return a.start < b.start;
                });

            //  Append in index by size
            free_intervals_by_size[interval.size].push_back(interval);

            // Update min/max
            if (interval.size < min_block_size) min_block_size = interval.size;
            if (interval.size > max_block_size) max_block_size = interval.size;
        }

        // Remove interval from indices
        void remove_interval(const HeapFreeInterval& interval) {
            // Find and remove from list by address
            auto node = free_intervals_by_addr.find_if(
                [&](const HeapFreeInterval& block) {
                    return block.start == interval.start &&
                        block.end == interval.end;
                });
            if (node) {
                free_intervals_by_addr.remove(node);
            }

            // Remove from index by size
            auto it = free_intervals_by_size.find(interval.size);
            if (it != free_intervals_by_size.end()) {
                auto& list = it->second;
                auto size_node = list.find_if(
                    [&](const HeapFreeInterval& block) {
                        return block.start == interval.start;
                    });
                if (size_node) {
                    list.remove(size_node);
                }

                // If list began empty -> delete writting
                if (list.empty()) {
                    free_intervals_by_size.erase(it);
                }
            }
        }

        // Find necessary block for allocation
        // Returns pointer on node with block or nullptr
        std::shared_ptr<typename LinkedList<HeapFreeInterval>::LinkedNode>
            find_fit_block(uint64_t required_size, HeapFreeInterval* out_block = nullptr) {
            // Search nearest necessary size
            for (size_t size = required_size; size <= max_block_size; ++size) {
                auto it = free_intervals_by_size.find(size);
                if (it != free_intervals_by_size.end() && !it->second.empty()) {
                    // Take first block by this size
                    auto node = it->second.get_front();
                    if (out_block) {
                        *out_block = node->data;
                    }
                    return node;
                }
            }
            return nullptr;
        }

        // Allocation with alignment
        // Returns ALLOC_SUCCESS/ALLOC_FAILURE
        // address - exit parameter
        int allocate(uint64_t size, uint64_t* address) {
            if (size == 0 || address == nullptr) {
                return ALLOC_FAILURE;
            }

            // Align the size
            uint64_t aligned_size = ((size + alignment - 1) / alignment) * alignment;

            // Find necessary block
            HeapFreeInterval block;
            auto node = find_fit_block(aligned_size, &block);
            if (node == nullptr) {
                return ALLOC_FAILURE; // Necessary block doesnt exist
            }

            *address = block.start;

            // Split block
            HeapFreeInterval allocated_part, remaining_part;
            int has_remaining = block.split(aligned_size, &allocated_part, &remaining_part);

            // Remove old block from indices
            remove_interval(block);

            // If remainder exists -> append it back
            if (has_remaining) {
                add_interval(remaining_part);
            }

            return ALLOC_SUCCESS;
        }

        // Allocation with specific alignment
        int allocate_aligned(uint64_t size, size_t specific_alignment, uint64_t* address) {
            if (size == 0 || address == nullptr) return ALLOC_FAILURE;

            size_t actual_alignment = alignment > specific_alignment ? alignment : specific_alignment;

            // Go by all blocks, find necessary with alignment
            auto node = free_intervals_by_addr.get_front();
            while (node) {
                uint64_t block_start = node->data.start;
                uint64_t block_size = node->data.size;

                // Calculating the aligned address within a block 
                uint64_t aligned_start = ((block_start + actual_alignment - 1)
                    / actual_alignment) * actual_alignment;

                // We check if the block fits with this alignment.
                if (aligned_start + size - 1 <= node->data.end) {
                    uint64_t required_total_size = aligned_start - block_start + size;

                    if (block_size >= required_total_size) {
                        *address = aligned_start;

                        // Remove block from indices
                        remove_interval(node->data);

                        // Create blocks for part before and after aligned area
                        if (aligned_start > block_start) {
                            // Front part
                            HeapFreeInterval front_block(block_start, aligned_start - 1);
                            add_interval(front_block);
                        }

                        if (aligned_start + size - 1 < node->data.end) {
                            // Back part
                            HeapFreeInterval back_block(aligned_start + size, node->data.end);
                            add_interval(back_block);
                        }

                        // Merge adjacent (смежные) blocks
                        coalesce();

                        return ALLOC_SUCCESS;
                    }
                }

                node = node->next;
            }

            return ALLOC_FAILURE;
        }

        // Memory freeing
        int free(uint64_t address, uint64_t size) {
            if (size == 0) return FREE_FAILURE;

            // Align size
            uint64_t aligned_size = ((size + alignment - 1) / alignment) * alignment;
            uint64_t end = address + aligned_size - 1;

            // Checking of exit beyond heap borders
            if (end < address) return FREE_FAILURE; // Overflow

            // Create new free block
            HeapFreeInterval free_block(address, end);

            // Append it in indices
            add_interval(free_block);

            // Merge adjacent (смежные) blocks
            coalesce();

            return FREE_SUCCESS;
        }

        // Merge adjacent (смежные) blocks
        void coalesce() {
            free_intervals_by_addr.coalesce(
                // Checking: can merge?
                [](const HeapFreeInterval& a, const HeapFreeInterval& b) {
                    return a.can_merge_with(b);
                },
                // Merge function
                [](const HeapFreeInterval& a, const HeapFreeInterval& b) {
                    return a.merge_with(b);
                }
            );

            // After coalescing we need update index by size
            rebuild_size_index();
        }

        // Rebuild indices by size after coalescing
        void rebuild_size_index() {
            free_intervals_by_size.clear();
            min_block_size = SIZE_MAX;
            max_block_size = 0;

            auto node = free_intervals_by_addr.get_front();
            while (node) {
                free_intervals_by_size[node->data.size].push_back(node->data);

                if (node->data.size < min_block_size) min_block_size = node->data.size;
                if (node->data.size > max_block_size) max_block_size = node->data.size;

                node = node->next;
            }
        }

        // Get statistic
        struct Stats {
            size_t total_blocks;
            uint64_t total_memory;
            size_t min_size;
            size_t max_size;
            size_t different_sizes;
        };

        Stats get_stats() const {
            Stats stats = { 0, 0, SIZE_MAX, 0, free_intervals_by_size.size() };

            auto node = free_intervals_by_addr.get_front();
            while (node) {
                stats.total_blocks++;
                stats.total_memory += node->data.size;

                if (node->data.size < stats.min_size) stats.min_size = node->data.size;
                if (node->data.size > stats.max_size) stats.max_size = node->data.size;

                node = node->next;
            }

            return stats;
        }

        // For debug
        void debug_print() const {
            std::cout << "=== Heap Free Intervals ===\n";
            std::cout << "Alignment: " << alignment << "\n";

            auto stats = get_stats();
            std::cout << "Blocks: " << stats.total_blocks
                << ", Memory: " << stats.total_memory << " bytes\n";
            std::cout << "Min block: " << stats.min_size
                << ", Max block: " << stats.max_size
                << ", Different sizes: " << stats.different_sizes << "\n\n";

            std::cout << "By address (sorted):\n";
            auto node = free_intervals_by_addr.get_front();
            while (node) {
                std::cout << "  ";
                node->data.print();
                std::cout << "\n";
                node = node->next;
            }

            std::cout << "\nBy size:\n";
            for (const auto& [size, list] : free_intervals_by_size) {
                std::cout << "  Size " << size << ": " << list.size() << " blocks\n";
            }
        }
    };

    // Global function for compatibility with existing code
    inline bool allocate_memory(HeapFreeIntervals& hfi, uint64_t& address, size_t size) {
        return hfi.allocate(size, &address) == ALLOC_SUCCESS;
    }

    inline bool free_memory(HeapFreeIntervals& hfi, uint64_t address, size_t size) {
        return hfi.free(address, size) == FREE_SUCCESS;
    }

    inline bool allocate_memory_aligned(HeapFreeIntervals& hfi, uint64_t& address,
        size_t size, size_t alignment) {
        return hfi.allocate_aligned(size, alignment, &address) == ALLOC_SUCCESS;
    }
}