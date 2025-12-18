#pragma once
#include "vm_definitions.hpp"
#include <stdexcept>
#include <memory>
#include <functional>




namespace MSLVM 
{
	
    template<typename T>
    class LinkedList
    {
    public:
        struct LinkedNode
        {
            T data;
            std::shared_ptr<LinkedNode> next;
            std::shared_ptr<LinkedNode> prev;

            LinkedNode(const T& d) : data(d), next(nullptr), prev(nullptr) {}
        };

    private:
        std::shared_ptr<LinkedNode> front;
        std::shared_ptr<LinkedNode> back;
        std::shared_ptr<LinkedNode> node_pointer;
        size_t size_;

    public:
        LinkedList() : front(nullptr), back(nullptr), node_pointer(nullptr), size_(0) {}

        LinkedList(const T& data) : size_(1)
        {
            front = std::make_shared<LinkedNode>(data);
            back = front;
            node_pointer = front;
        }

        // Inserting before current node
        std::shared_ptr<LinkedNode> insert_before(std::shared_ptr<LinkedNode> node, const T& data)
        {
            if (!node) return push_back(data);

            auto new_node = std::make_shared<LinkedNode>(data);
            new_node->next = node;
            new_node->prev = node->prev;

            if (node->prev) {
                node->prev->next = new_node;
            }
            else {
                front = new_node;  // inserting to the start
            }

            node->prev = new_node;
            size_++;
            return new_node;
        }

        // Inserting after current node
        std::shared_ptr<LinkedNode> insert_after(std::shared_ptr<LinkedNode> node, const T& data)
        {
            if (!node) return push_front(data);

            auto new_node = std::make_shared<LinkedNode>(data);
            new_node->prev = node;
            new_node->next = node->next;

            if (node->next) {
                node->next->prev = new_node;
            }
            else {
                back = new_node;  //inserting to the end
            }

            node->next = new_node;
            size_++;
            return new_node;
        }

        // Inserting to the start
        std::shared_ptr<LinkedNode> push_front(const T& data)
        {
            auto new_node = std::make_shared<LinkedNode>(data);

            if (front) {
                new_node->next = front;
                front->prev = new_node;
                front = new_node;
            }
            else {
                front = new_node;
                back = front;
                node_pointer = front;
            }

            size_++;
            return new_node;
        }

        //Inserting to the end
        std::shared_ptr<LinkedNode> push_back(const T& data)
        {
            auto new_node = std::make_shared<LinkedNode>(data);

            if (back) {
                new_node->prev = back;
                back->next = new_node;
                back = new_node;
            }
            else {
                front = new_node;
                back = front;
                node_pointer = front;
            }

            size_++;
            return new_node;
        }

        // Node's removing
        void remove(std::shared_ptr<LinkedNode> node)
        {
            if (!node) return;

            // Update neightbors pointers
            if (node->prev) {
                node->prev->next = node->next;
            }
            else {
                front = node->next;  // remove first
            }

            if (node->next) {
                node->next->prev = node->prev;
            }
            else {
                back = node->prev;  // remove last 
            }

            // Update node_pointer if he points on deleted node
            if (node_pointer == node) {
                node_pointer = node->next ? node->next : node->prev;
            }

            size_--;
        }

        // Search by condition
        template<typename Predicate>
        std::shared_ptr<LinkedNode> find_if(Predicate pred)
        {
            auto current = front;
            while (current) {
                if (pred(current->data)) {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        // Sort inserting (for free list - by address)
        std::shared_ptr<LinkedNode> insert_sorted(const T& data,
            std::function<bool(const T&, const T&)> comparator)
        {
            // If list is empty or element must be first
            if (!front || comparator(data, front->data)) {
                return push_front(data);
            }

            // Search place for inserting
            auto current = front;
            while (current->next && comparator(current->next->data, data)) {
                current = current->next;
            }

            // Insert after current
            return insert_after(current, data);
        }

        // Merge neighbor nodes(for free list)
        void coalesce(std::function<bool(const T&, const T&)> can_merge,
            std::function<T(const T&, const T&)> merge_func)
        {
            auto current = front;
            while (current && current->next) {
                if (can_merge(current->data, current->next->data)) {
                    // Merge two nodes to one 
                    current->data = merge_func(current->data, current->next->data);
                    remove(current->next);
                    //Dont move current - check again on merge opportunity
                }
                else {
                    current = current->next;
                }
            }
        }

        // Useful getters
        size_t size() const { return size_; }
        bool empty() const { return size_ == 0; }
        std::shared_ptr<LinkedNode> get_front() const { return front; }
        std::shared_ptr<LinkedNode> get_back() const { return back; }
        std::shared_ptr<LinkedNode> get_current() const { return node_pointer; }

        // Iteration
        void reset_pointer() { node_pointer = front; }
        bool move_forward()
        {
            if (!node_pointer || !node_pointer->next) return false;
            node_pointer = node_pointer->next;
            return true;
        }

        bool move_backward()
        {
            if (!node_pointer || !node_pointer->prev) return false;
            node_pointer = node_pointer->prev;
            return true;
        }
    };

}