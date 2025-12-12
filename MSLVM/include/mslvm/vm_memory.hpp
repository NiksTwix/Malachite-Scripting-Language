#pragma once
#include <cstdint>
#include "vm_definitions.hpp"

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

}