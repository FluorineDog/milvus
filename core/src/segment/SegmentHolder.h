#pragma once
#include <vector>

#include "db/Types.h"

namespace milvus {
namespace engine {

using Timestamp = int64_t;  // TODO
struct IndexParam {};       // TODO
struct FieldsInfo {};       // TODO

class SegmentHolder {
 public:
    using Id = IDNumbers;
    SegmentHolder(std::shared_ptr<FieldsInfo> collection);

    // data_chunk contains ids
    Status
    AppendData(const DataChunkPtr& data_chunk);

    Status
    DeleteEntityByIds(Timestamp timestamp, const std::vector<Id>& ids);

    Status
    Query(const query::QueryPtr& query, std::vector<Id>& results, Timestamp timestamp = 0);

    Status
    GetEntityByIds(Timestamp timestamp, const std::vector<Id>& ids, DataChunkPtr& results);

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
    BuildIndex(std::shared_ptr<IndexParam> index_params);

    bool
    DropIndex();

 public:
    // getter and setters
    size_t
    get_row_count() const;

    const FieldsInfo&
    get_fields_info() const;

    const IndexParam&
    get_index_param() const;

    bool
    is_indexed() const;

 public:
    enum class SegmentState {
        Invalid = 0,
        Insert,  // able to insert data
        Frozen   // able to build index
    };

 private:
    std::mutex local_mutex_;
    std::atomic<SegmentState> state_ = SegmentState::Invalid;
    std::shared_ptr<FieldsInfo> fields_info_;
    std::shared_ptr<IndexParam> index_param_;

    // TODO: data holders
};

}  // namespace engine
}  // namespace milvus
