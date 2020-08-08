#pragma once
#include <vector>

#include "db/Types.h"
#include "knowhere/index/Index.h"

namespace milvus {
namespace engine {

using Timestamp = uint64_t;  // TODO: use TiKV-like timestamp

struct IndexConfig {
    // TODO
    std::unordered_map<std::string, knowhere::Config> configs;
};

struct FieldsInfo {
    // TODO: add basic operations
    std::unordered_map<std::string, Field> fields;
};

class SegmentHolder : public cache::DataObj {
 public:
    // definitions

    using Id = IDNumbers;
    enum class SegmentState {
        Invalid = 0,
        Insert,  // able to insert data
        Frozen   // able to build index
    };

 public:
    SegmentHolder(std::shared_ptr<FieldsInfo> collection);

    //
    Status
    Insert(Timestamp timestamp, const DataChunkPtr& data_chunk, const std::vector<Id>& ids);

    Status
    DeleteEntityByIds(Timestamp timestamp, const std::vector<Id>& ids);

    // query contains metadata of
    Status
    Query(const query::QueryPtr& query, std::vector<Id>& results, Timestamp timestamp = 0);

    //    // THIS FUNCTION IS REMOVED
    //    Status
    //    GetEntityByIds(Timestamp timestamp, const std::vector<Id>& ids, DataChunkPtr& results);

    // stop receive insert requests
    Status
    Freeze();

    // to make all data inserted visible
    // maybe a no-op
    Status
    Flush(Timestamp timestamp);

    // BuildIndex With Paramaters, must with Frozen State
    // This function is atomic
    Status
    BuildIndex(std::shared_ptr<IndexConfig> index_params, std::string_view field_name);

    bool
    DropIndex();

    Status
    Serilize(std::string_view root);

    Status
    Deserilize(std::string_view root);

 public:
    // getter and setters

    size_t
    get_row_count() const;

    const FieldsInfo&
    get_fields_info() const;

    // check is_indexed here
    const IndexConfig&
    get_index_param() const;

    SegmentState
    get_state() const;

    uint32_t
    get_max_timestamp();

 private:
    std::shared_mutex meta_mutex_;
    std::atomic<SegmentState> state_ = SegmentState::Invalid;
    std::shared_ptr<FieldsInfo> fields_info_;
    std::shared_ptr<IndexConfig> index_param_;

    // we are holding data there
    // TODO: should we split index into vector and scalar?
    std::unordered_map<std::string, knowhere::IndexPtr> indexes_;

    // TODO: data holders
};

}  // namespace engine
}  // namespace milvus
