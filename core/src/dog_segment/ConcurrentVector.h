#pragma once
#include <tbb/concurrent_vector.h>

#include <vector>
namespace milvus::dog_segment {


// we don't use std::array because capacity of concurrent_vector wastes too much memory
template<typename Type, int Size>
class FixedVector: public std::vector<Type> {
 public:
    FixedVector(): std::vector<Type>(Size) {}
};

template<typename Type, int Dim>
class ConcurrentVector {
 public:
    // constants
    static constexpr ssize_t ElementsPerChunk = 32 * 1024; // TODO: Configuarable
    static constexpr ssize_t DataPerChunk = Dim * ElementsPerChunk;
    using Chunk = FixedVector<Type, DataPerChunk>;
 public:

 private:
    tbb::concurrent_vector<Chunk> chunks_;
};
}