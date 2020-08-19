#include <shared_mutex>

#include "dog_segment/SegmentBase.h"
#include "query/GeneralQuery.h"
#include "utils/Status.h"

namespace milvus::engine {

int
TestABI() {
    return 42;
}

class SegmentNaive : public SegmentBase {
 public:
    virtual ~SegmentNaive() = default;
    // SegmentBase(std::shared_ptr<FieldsInfo> collection);

    // TODO: originally, id should be put into data_chunk
    // TODO: Is it ok to put them the other side?
    Status
    Insert(const std::vector<id_t>& primary_keys, const std::vector<Timestamp>& timestamps,
           DogDataChunkPtr values) override {
        throw std::runtime_error("not implemented");
        return Status::OK();
    }

    // TODO: add id into delete log, possibly bitmap
    Status
    Delete(const std::vector<id_t>& primary_keys, const std::vector<Timestamp>& timestamps) override {
        throw std::runtime_error("not implemented");
        return Status::OK();
    }

    // query contains metadata of
    Status
    Query(const query::QueryPtr& query, Timestamp timestamp, QueryResult& results) override {
        throw std::runtime_error("not implemented");
        return Status::OK();
    };

    // // THIS FUNCTION IS REMOVED
    // virtual Status
    // GetEntityByIds(Timestamp timestamp, const std::vector<Id>& ids, DataChunkPtr& results) = 0;

    // stop receive insert requests
    Status
    Close() override {
        std::lock_guard<std::shared_mutex> lck(mutex_);
        assert(state_ == SegmentState::Open);
        state_ = SegmentState::Closed;
        return Status::OK();
    }

    //    // to make all data inserted visible
    //    // maybe a no-op?
    //    virtual Status
    //    Flush(Timestamp timestamp) = 0;

    // BuildIndex With Paramaters, must with Frozen State
    // This function is atomic
    // NOTE: index_params contains serveral policies for several index
    Status
    BuildIndex(std::shared_ptr<IndexConfig> index_params) override {
        throw std::runtime_error("not implemented");
    }

    // Remove Index
    Status
    DropIndex(const std::string& field_name) override {
        throw std::runtime_error("not implemented");
    }

    Status
    DropRawData(const std::string& field_name) override {
        // TODO: NO-OP
        return Status::OK();
    }

    Status
    LoadRawData(const std::string& field_name, const char* blob, int64_t blob_size) override {
        // TODO: NO-OP
        return Status::OK();
    }

 public:
    ssize_t
    get_row_count() const override {
        return count_.load(std::memory_order_relaxed);
    }

    //    const FieldsInfo&
    //    get_fields_info() const override {
    //
    //    }
    //
    //    // check is_indexed here
    //    virtual const IndexConfig&
    //    get_index_param() const = 0;
    //
    SegmentState
    get_state() const override {
        return state_.load(std::memory_order_relaxed);
    }
    //
    //    std::shared_ptr<IndexData>
    //    get_index_data();

    Timestamp
    get_max_timestamp() override {
        return 0;
    }

    Timestamp
    get_min_timestamp() override {
        return 0;
    }

    ssize_t
    get_deleted_count() const override {
        return 0;
    }

 public:
 private:
    std::shared_mutex mutex_;
    std::atomic<SegmentState> state_ = SegmentState::Open;
    std::atomic<int64_t> count_ = 0;
    //    std::shared_ptr<FieldsInfo> fields_info_;
    //    std::shared_ptr<IndexConfig> index_param_;

    //    // we are holding data there
    //    // TODO: should we split index into vector and scalar?
    //    std::unordered_map<std::string, knowhere::IndexPtr> indexes_;

    //     TODO: data holders
};

std::shared_ptr<SegmentBase>
CreateSegment() {
    auto ptr = std::make_shared<SegmentNaive>();

    return ptr;
}

}  // namespace milvus::engine
