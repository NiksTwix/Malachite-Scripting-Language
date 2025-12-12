#pragma once
#include "vm_definitions.hpp"
#include <stdexcept>

namespace MSLVM
{

	template<typename T, size_t MAX_SIZE>
    class VMStack {
    private:
        T m_data[MAX_SIZE];
        size_t m_top = 0;

    public:
        void push(const T& value) {
            if (m_top >= MAX_SIZE) {
                throw std::runtime_error("Malachite: Stack overflow");
            }
            m_data[m_top++] = value;
        }

        T pop() {
            if (m_top == 0) {
                throw std::runtime_error("Malachite: Stack underflow");
            }
            return m_data[--m_top];
        }


        T& top() {
            if (m_top == 0) throw std::runtime_error("Malachite: Stack empty");
            return m_data[m_top - 1];
        }

        const T& top() const {
            if (m_top == 0) throw std::runtime_error("Malachite: Stack empty");
            return m_data[m_top - 1];
        }

        const T& at(size_t index) const 
        {
            if (m_top == 0 || index >= size()) throw std::runtime_error("Malachite: Stack index " + std::to_string(index) + " out of range (size=" + std::to_string(size()) + ")");
            return m_data[index];
        }

        size_t size() const { return m_top; }
        bool empty() const { return m_top == 0; }
        void clear() { m_top = 0; }

        // Итераторы для отладки
        const T* begin() const { return m_data; }
        const T* end() const { return m_data + m_top; }
    };
}