#include
class SegmentBase {
 public:
    // definitions
    enum class SegmentState {
        Invalid = 0,
        Inserting,  // able to insert data
        Frozen      // able to build index
    };
 public:
    virtual ~SegmentBase() = default;
    // SegmentBase(std::shared_ptr<FieldsInfo> collection);

    // TODO: originally, id should be put into data_chunk
    // TODO: Is it ok to put them the other side?
    virtual Status
    Insert(Timestamp timestamp, DataChunkPtr data_chunk) = 0;

    // TODO: add id into delete log, possibly bitmap
    virtual Status
    DeleteEntityByIds(std::vector<Timestamp> timestamp, const std::vector<id_t>& ids) = 0;

    // query contains metadata of
    virtual Status
    Query(const query::QueryPtr& query, std::vector<id_t>& results, Timestamp timestamp = 0) = 0;

    // // THIS FUNCTION IS REMOVED
    // virtual Status
    // GetEntityByIds(Timestamp timestamp, const std::vector<Id>& ids, DataChunkPtr& results) = 0;

    // stop receive insert requests
    virtual Status
    Freeze() = 0;

    //    // to make all data inserted visible
    //    // maybe a no-op?
    //    virtual Status
    //    Flush(Timestamp timestamp) = 0;

    // BuildIndex With Paramaters, must with Frozen State
    // This function is atomic
    // NOTE: index_params contains serveral policies for several index
    virtual Status
    BuildIndex(std::shared_ptr<IndexConfig> index_params, std::shared_ptr<IndexData> index_data = nullptr) = 0;

    // Remove Index
    virtual Status
    DropIndex(std::vector<std::string> fields) = 0;

 public:
    virtual ssize_t
    get_row_count() const = 0;

    virtual const FieldsInfo&
    get_fields_info() const = 0;

    // check is_indexed here
    virtual const IndexConfig&
    get_index_param() const = 0;

    virtual SegmentState
    get_state() const = 0;

    std::shared_ptr<IndexData>
    get_index_data();

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