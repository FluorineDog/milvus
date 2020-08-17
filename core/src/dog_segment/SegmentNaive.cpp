#include "dog_segment/SegmentHolder.h"
#include "query/GeneralQuery.h"
#include "utils/Status.h"

namespace milvus::engine {

std::shared_ptr<SegmentBase>
Create() {
    return nullptr;
}

class SegmentNaive : public SegmentBase {
 public:
    virtual ~SegmentNaive() = default;
    // SegmentBase(std::shared_ptr<FieldsInfo> collection);

    // TODO: originally, id should be put into data_chunk
    // TODO: Is it ok to put them the other side?
    Status
    Insert(Timestamp timestamp, DataChunkPtr data_chunk) override {
        return Status::OK();
    }

    // TODO: add id into delete log, possibly bitmap
    Status
    DeleteEntityByIds(std::vector<Timestamp> timestamp, const std::vector<id_t>& ids) override {
        return Status::OK();
    }

    // query contains metadata of
    Status
    Query(const query::QueryPtr& query, std::vector<id_t>& results, Timestamp timestamp = 0) override {
        return Status::OK();
    };

    // // THIS FUNCTION IS REMOVED
    // virtual Status
    // GetEntityByIds(Timestamp timestamp, const std::vector<Id>& ids, DataChunkPtr& results) = 0;

    // stop receive insert requests
    Status
    Freeze() override {
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
    BuildIndex(std::shared_ptr<IndexConfig> index_params, std::shared_ptr<IndexData> index_data = nullptr) override {
        return Status::OK();
    }

    // Remove Index
    Status
    DropIndex(std::vector<std::string> fields) override {
        return Status::OK();
    }

 public:
    ssize_t
    get_row_count() const override {
        return 0;
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
        return SegmentState::Invalid;
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
    //    std::shared_mutex mutex_;
    //    std::atomic<SegmentState> state_ = SegmentState::Invalid;
    //    std::shared_ptr<FieldsInfo> fields_info_;
    //    std::shared_ptr<IndexConfig> index_param_;

    //    // we are holding data there
    //    // TODO: should we split index into vector and scalar?
    //    std::unordered_map<std::string, knowhere::IndexPtr> indexes_;

    //     TODO: data holders
};
}  // namespace milvus::engine
