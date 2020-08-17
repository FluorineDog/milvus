#pragma once
#include "dog_segment/SegmentDefs.h"
#include <vector>

#include "db/Types.h"
#include "knowhere/index/Index.h"
#include "query/GeneralQuery.h"

namespace milvus {
namespace engine {

struct QueryResult {};

class SegmentBase {
 public:
    // definitions
    enum class SegmentState {
        Invalid = 0,
        Open,  // able to insert data
        Closed      // able to build index
    };
 public:
    virtual ~SegmentBase() = default;
    // SegmentBase(std::shared_ptr<FieldsInfo> collection);

    virtual Status
    Insert(const std::vector<id_t>& primary_keys, const std::vector<Timestamp>& timestamps, DogDataChunkPtr values) = 0;

    // TODO: add id into delete log, possibly bitmap
    virtual Status
    Delete(const std::vector<id_t>& primary_keys, const std::vector<Timestamp>& timestamps) = 0;

    // query contains metadata of
    virtual Status
    Query(const query::QueryPtr& query, Timestamp timestamp, std::vector<QueryResult>& results) = 0;

    // // THIS FUNCTION IS REMOVED
    // virtual Status
    // GetEntityByIds(Timestamp timestamp, const std::vector<Id>& ids, DataChunkPtr& results) = 0;

    // stop receive insert requests
    virtual Status
    Close() = 0;

    //    // to make all data inserted visible
    //    // maybe a no-op?
    //    virtual Status
    //    Flush(Timestamp timestamp) = 0;


    // BuildIndex With Paramaters, must with Frozen State
    // This function is atomic
    // NOTE: index_params contains serveral policies for several index
    virtual Status
    BuildIndex(std::shared_ptr<IndexConfig> index_params) = 0;

    // Remove Index
    virtual Status
    DropIndex(const std::string& field_name) = 0;

    virtual Status
    DropRawData(const std::string& field_name) = 0;

    virtual Status
    LoadRawData(const std::string& field_name, const char* blob, int64_t blob_size) = 0;


 
 public:
    virtual ssize_t
    get_row_count() const = 0;

//    virtual const FieldsInfo&
//    get_fields_info() const = 0;
//
//    // check is_indexed here
//    virtual const IndexConfig&
//    get_index_param() const = 0;
//
    virtual SegmentState
    get_state() const = 0;
//
//    std::shared_ptr<IndexData>
//    get_index_data();

    virtual Timestamp
    get_max_timestamp() = 0;

    virtual Timestamp
    get_min_timestamp() = 0;

    virtual ssize_t
    get_deleted_count() const = 0;

 public:
 private:
    //    std::shared_mutex mutex_;
    //    std::atomic<SegmentState> state_ = SegmentState::Invalid;
    //    std::shared_ptr<FieldsInfo> fields_info_;
    //    std::shared_ptr<IndexConfig> index_param_;

    //    // we are holding data there
    //    // TODO: should we split index into vector and scalar?
    //    std::unordered_map<std::string, knowhere::IndexPtr> indexes_;

    //     TODO: data holders
};

std::shared_ptr<SegmentBase> CreateSegment(/*args*/);

}  // namespace engine
}  // namespace milvus
