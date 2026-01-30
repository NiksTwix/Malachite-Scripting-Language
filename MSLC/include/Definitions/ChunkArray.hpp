#pragma once
#include <vector>
#include <stdexcept>
#include <string>
namespace MSLC 
{
	namespace Definitions 
	{
        constexpr size_t CHUNK_SIZE = 256;


        template<typename T>
        struct Chunk {
            T* begin;
            T* end;

            Chunk() {
                begin = new T[CHUNK_SIZE];
                end = begin + CHUNK_SIZE;
            }

            ~Chunk() {
                delete[] begin;
                begin = nullptr;
                end = nullptr;
            }


            Chunk(const Chunk& other)
            {
                begin = new T[CHUNK_SIZE];
                end = begin + CHUNK_SIZE;

                if constexpr (std::is_trivially_copyable_v<T>)memcpy(begin, other.begin, CHUNK_SIZE * sizeof(T));
                else std::copy(other.begin, other.end, begin);  //begin-end is interval, begin is destination
            }
            Chunk& operator=(const Chunk& other) {      //other is right operand
                if (this != &other) { 
                    begin = new T[CHUNK_SIZE];
                    end = begin + CHUNK_SIZE;
                    if constexpr (std::is_trivially_copyable_v<T>)memcpy(begin, other.begin, CHUNK_SIZE * sizeof(T));
                    else std::copy(other.begin, other.end, begin);  //begin-end is interval, begin is destination
                }
                return *this;
            }

            // Permit moving
            Chunk(Chunk&& other) noexcept
                : begin(other.begin), end(other.end) {
                other.begin = nullptr;
                other.end = nullptr;
            }

            Chunk& operator=(Chunk&& other) noexcept {
                if (this != &other) {
                    delete[] begin;
                    begin = other.begin;
                    end = other.end;
                    other.begin = nullptr;
                    other.end = nullptr;
                }
                return *this;
            }

            T& operator[](size_t index) { return begin[index]; }
            const T& operator[](size_t index) const { return begin[index]; }

            size_t size() const { return CHUNK_SIZE; }

            friend void swap(Chunk& a, Chunk& b) noexcept {
                using std::swap;
                swap(a.begin, b.begin);
                swap(a.end, b.end);
            }
        };

        template<typename T>
        class alignas (8) ChunkArray {
            std::vector<Chunk<T>> chunks;
            size_t size_ = 0;

        public:
            
            ~ChunkArray() 
            {
                Clear();
            }

            ChunkArray(const ChunkArray& other) : size_(0) {  
                Reserve(other.size_);

                for (size_t i = 0; i < other.size_; ++i) {
                    size_t chunk_idx = i / CHUNK_SIZE;
                    size_t elem_idx = i % CHUNK_SIZE;
                    Pushback(other.chunks[chunk_idx][elem_idx]);
                }
            }

            ChunkArray<T>& operator=(const ChunkArray<T>& other)
            {
                if (&other == this) return *this;
                if (size_ != 0)Clear();
                Reserve(other.size_);
                for (size_t i = 0; i < other.size_; ++i) {
                    size_t chunk_idx = i / CHUNK_SIZE;
                    size_t elem_idx = i % CHUNK_SIZE;
                    
                    const T& src_elem = other.chunks[chunk_idx][elem_idx];
                    
                        Pushback(src_elem);
                }
                return *this;
            }

            ChunkArray(ChunkArray&& other) noexcept
                : chunks(std::move(other.chunks))
                , size_(other.size_) {
                other.size_ = 0;
            }

            ChunkArray& operator=(ChunkArray&& other) noexcept {
                if (this != &other) {
                    chunks = std::move(other.chunks);
                    size_ = other.size_;
                    other.size_ = 0;
                }
                return *this;
            }
            ChunkArray() 
            {
                size_ = 0;
                chunks = std::vector<Chunk<T>>();
            }

            explicit ChunkArray(const std::vector<T>& vec) {
                Reserve(vec.size());
                for (const T& item : vec) {
                    Pushback(item);
                }
            }

            //moving
            explicit ChunkArray(std::vector<T>&& vec) {
                Reserve(vec.size());
                for (T& item : vec) {
                    Pushback(std::move(item));
                }
                // vector will be self-cleared  
            }


            // Main methods
            T& Get(size_t index) {
                if (index >= size_) {
                    throw std::out_of_range("Index out of bounds");
                }

                size_t chunk_index = index / CHUNK_SIZE;
                size_t index_in_chunk = index % CHUNK_SIZE; 

                return chunks[chunk_index][index_in_chunk];
            }

            const T& Get(size_t index) const {
                if (index >= size_) {
                    throw std::out_of_range("Index out of bounds");
                }

                size_t chunk_index = index / CHUNK_SIZE;
                size_t index_in_chunk = index % CHUNK_SIZE;

                return chunks[chunk_index][index_in_chunk];
            }


            T& Front() {
                if (size_ == 0) {
                    throw std::out_of_range("Chunk array is empty");
                }
                return chunks[0][0];
            }

            const T& Front() const {
                if (size_ == 0) {
                    throw std::out_of_range("Chunk array is empty");
                }
                return chunks[0][0];
            }

            T& Back() {
                if (size_ == 0) {
                    throw std::out_of_range("Chunk array is empty");
                }
                size_t chunk_index = (size_ - 1) / CHUNK_SIZE;
                size_t index_in_chunk = (size_ - 1) % CHUNK_SIZE;
                return chunks[chunk_index][index_in_chunk];
            }

            const T& Back() const {
                if (size_ == 0) {
                    throw std::out_of_range("Chunk array is empty");
                }
                size_t chunk_index = (size_-1) / CHUNK_SIZE;
                size_t index_in_chunk = (size_ - 1) % CHUNK_SIZE;
                return chunks[chunk_index][index_in_chunk];
            }
            // Operators [] overloadings
            T& operator[](size_t index) { return Get(index); }
            const T& operator[](size_t index) const { return Get(index); }

            void Set(size_t index, const T& element) {
                if (index >= size_) {
                    throw std::out_of_range("Index out of bounds");
                }

                size_t chunk_index = index / CHUNK_SIZE;
                size_t index_in_chunk = index % CHUNK_SIZE;

                chunks[chunk_index][index_in_chunk] = element;
            }

            void Overwrite(ChunkArray<T>& other, size_t where_index) {
                if (where_index > size_) throw std::out_of_range("Index out of bounds");  //If where_index == size_ -> analog pushback, insertion without overwritting
                //If where_index == size_ -> appends to the end
                size_t required_size = where_index + other.size_;

                if (required_size > size_) {
                    size_ = required_size;
                    Reserve(size_);
                }

                for (size_t i = 0; i < other.size_; i++) {
                    size_t chunk_index = (where_index + i) / CHUNK_SIZE;
                    size_t index_in_chunk = (where_index + i) % CHUNK_SIZE;

                    size_t other_chunk_index = i / CHUNK_SIZE;
                    size_t other_index_in_chunk = i % CHUNK_SIZE;

                    chunks[chunk_index][index_in_chunk] =
                        other.chunks[other_chunk_index][other_index_in_chunk];
                }
            }
           
            void Overwrite(ChunkArray<T>&& other, size_t where_index) {
                if (where_index > size_) throw std::out_of_range("Index out of bounds");    //If where_index == size_ -> analog pushback, insertion without overwritting
                //If where_index == size_ -> appends to the end
                size_t required_size = where_index + other.size_;

                if (required_size > size_) {
                    size_ = required_size;
                    Reserve(size_);
                }

                for (size_t i = 0; i < other.size_; i++) {
                    size_t chunk_index = (where_index + i) / CHUNK_SIZE;
                    size_t index_in_chunk = (where_index + i) % CHUNK_SIZE;

                    size_t other_chunk_index = i / CHUNK_SIZE;
                    size_t other_index_in_chunk = i % CHUNK_SIZE;

                    chunks[chunk_index][index_in_chunk] =
                        std::move(other.chunks[other_chunk_index][other_index_in_chunk]);
                }
                other.Clear();
            }

            void OverwriteBack(ChunkArray<T>& other) {
                if (other.size_ <= size_) {
                    //Replace tail
                    for (size_t i = 0; i < other.size_; ++i) {
                        size_t dst_index = size_ - other.size_ + i;
                        size_t dst_chunk = dst_index / CHUNK_SIZE;
                        size_t dst_elem = dst_index % CHUNK_SIZE;

                        size_t src_chunk = i / CHUNK_SIZE;
                        size_t src_elem = i % CHUNK_SIZE;

                        chunks[dst_chunk][dst_elem] = other.chunks[src_chunk][src_elem];
                    }
                }
                else {
                    // Append to the end (expand)
                    Overwrite(other, size_);
                }
            }
            void OverwriteBack(ChunkArray<T>&& other) {
                if (other.size_ <= size_) {
                    //Replace tail
                    for (size_t i = 0; i < other.size_; ++i) {
                        size_t dst_index = size_ - other.size_ + i;
                        size_t dst_chunk = dst_index / CHUNK_SIZE;
                        size_t dst_elem = dst_index % CHUNK_SIZE;

                        size_t src_chunk = i / CHUNK_SIZE;
                        size_t src_elem = i % CHUNK_SIZE;

                        chunks[dst_chunk][dst_elem] = other.chunks[src_chunk][src_elem];
                    }
                }
                else {
                    // Append to the end (expand)
                    Overwrite(std::move(other), size_);
                }
            }
            void Pushback(ChunkArray<T>& other)
            {
                for (size_t i = 0; i < other.size_; i++)
                {
                    size_t chunk_index = (size_ + i) / CHUNK_SIZE;
                    size_t index_in_chunk = (size_ + i) % CHUNK_SIZE;
                    size_t other_chunk_index = i / CHUNK_SIZE;
                    size_t other_index_in_chunk = i % CHUNK_SIZE;

                    chunks[chunk_index][index_in_chunk] = other.chunks[other_chunk_index][other_index_in_chunk];
                }
                size_ += other.size_;
            }
            void Pushback(ChunkArray<T>&& other) {
                for (size_t i = 0; i < other.size_; i++) {
                    size_t chunk_index = (size_ + i) / CHUNK_SIZE;
                    size_t index_in_chunk = (size_ + i) % CHUNK_SIZE;
                    size_t other_chunk_index = i / CHUNK_SIZE;
                    size_t other_index_in_chunk = i % CHUNK_SIZE;

                    chunks[chunk_index][index_in_chunk] =
                        std::move(other.chunks[other_chunk_index][other_index_in_chunk]);
                }
                size_ += other.size_;
                other.Clear();  
            }
            void Pushback(const T& element) {
                size_t chunk_index = size_ / CHUNK_SIZE;

                if (chunk_index >= chunks.size()) {
                    chunks.emplace_back();
                }

                size_t index_in_chunk = size_ % CHUNK_SIZE;
                chunks[chunk_index][index_in_chunk] = element;
                size_++;
            }

            void Pushback(T&& element) {
                size_t chunk_index = size_ / CHUNK_SIZE;

                if (chunk_index >= chunks.size()) {
                    chunks.emplace_back();
                }

                size_t index_in_chunk = size_ % CHUNK_SIZE;
                chunks[chunk_index][index_in_chunk] = std::move(element);
                size_++;
            }

            void Popback() 
            {
                if (size_ == 0) throw std::out_of_range("Chunk array is empty, \"Popback\" hasnt executed");
                size_t last_index = size_ - 1;
                size_t chunk_index = last_index / CHUNK_SIZE;
                size_t index_in_chunk = last_index % CHUNK_SIZE;

                if constexpr (!std::is_pod_v<T>)
                {
                    chunks[chunk_index][index_in_chunk].~T();
                }
                if (index_in_chunk == 0) chunks.pop_back();
                size_--;
            }


            template<typename... Args>
            T& EmplaceBack(Args&&... args) {
                size_t chunk_index = size_ / CHUNK_SIZE;

                if (chunk_index >= chunks.size()) {
                    chunks.emplace_back();
                }

                size_t index_in_chunk = size_ % CHUNK_SIZE;
                T* ptr = &chunks[chunk_index][index_in_chunk];

                try {
                    new (ptr) T(std::forward<Args>(args)...);   // creates new object in allocated memory
                }
                catch (...) {
                    throw;
                }

                ++size_;
                return *ptr;
            }

            // Additional methods
            inline size_t Size() const { 
                return this->size_; 
            }
            inline bool Empty() const 
            { 
                return this->size_ == 0;
            }

            size_t Capacity() const {
                return chunks.size() * CHUNK_SIZE;
            }

            void Clear() {

                if constexpr (!std::is_pod_v<T>) {
                    for (size_t i = 0; i < size_; ++i) {
                        size_t chunk_idx = i / CHUNK_SIZE;
                        size_t elem_idx = i % CHUNK_SIZE;
                        chunks[chunk_idx][elem_idx].~T();
                    }
                }

                chunks.clear();
                size_ = 0;
            }

            void Reserve(size_t new_capacity) {
                size_t needed_chunks = (new_capacity + CHUNK_SIZE - 1) / CHUNK_SIZE;    /// top rounding by "+ CHUNK_SIZE - 1"
                while (chunks.size() < needed_chunks) {
                    chunks.emplace_back();
                }
            }

            // For debuging
            size_t ChunkCount() const { return chunks.size(); }
            size_t MemoryUsage() const {
                return chunks.size() * CHUNK_SIZE * sizeof(T);
            }
        };

	}
}