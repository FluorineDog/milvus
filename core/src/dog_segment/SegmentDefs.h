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

struct FieldInfo {
    std::string field_name;
    std::string field_id;
};

struct FieldsInfo {
    // TODO: add basic operations
    std::unordered_map<std::string, FieldElementType> fields;
};

struct FieldMeta {
 public:
    FieldMeta(std::string_view name, DataType type) : name_(name), type_(type) {
    }

    bool
    is_vector() const {
        assert(type_ != DataType::NONE);
        return type_ == DataType::VECTOR_BINARY || type_ == DataType::VECTOR_FLOAT;
    }

    void
    set_dim(int dim) {
        assert(type_ != DataType::VECTOR_BINARY || dim % 8 == 0);
        dim_ = dim;
    }

    size_t
    get_sizeof() const {
        switch (type_) {
            case DataType::BOOL:
                return sizeof(bool);
            case DataType::DOUBLE:
                return sizeof(double);
            case DataType::FLOAT:
                return sizeof(float);
            case DataType::INT8:
                return sizeof(uint8_t);
            case DataType::INT16:
                return sizeof(uint16_t);
            case DataType::INT32:
                return sizeof(uint32_t);
            case DataType::INT64:
                return sizeof(uint64_t);
            case DataType::VECTOR_FLOAT:
                return sizeof(float) * dim_;
            case DataType::VECTOR_BINARY: {
                assert(dim_ % 8 == 0);
                return dim_ / 8;

            }
            default: {
                assert(false);
                return 0;
            }
        }
    }

    std::string
    get_name() {
        return name_;
    }

 private:
    std::string name_;
    DataType type_ = DataType::NONE;
    int dim_ = 1;
};

struct Schema {
    std::vector<FieldMeta> field_metas;
};

using SchemaPtr = std::shared_ptr<Schema>;

class IndexData {
 public:
    virtual std::vector<char>
    serilize() = 0;

    static std::shared_ptr<IndexData>
    deserialize(int64_t size, const char* blob);
};

}  // namespace milvus::engine
