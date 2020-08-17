#pragma once


#include <vector>

#include "db/Types.h"
#include "knowhere/index/Index.h"
#include "utils/Status.h"

using Timestamp = uint64_t;  // TODO: use TiKV-like timestamp
namespace milvus::engine {

struct IndexConfig {
    // TODO
    std::unordered_map<std::string, knowhere::Config> configs;
};

struct FieldsInfo {
    // TODO: add basic operations
    std::unordered_map<std::string, FieldElementType> fields;
};

class IndexData {
 public:
    virtual std::vector<char>
    serilize() = 0;

    static std::shared_ptr<IndexData>
    deserialize(int64_t size, const char* blob);
};


}
