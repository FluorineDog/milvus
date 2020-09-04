#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_priority_queue.h>

#include <shared_mutex>

#include "dog_segment/SegmentBase.h"
#include "knowhere/index/structured_index/StructuredIndex.h"
#include "query/GeneralQuery.h"
#include "utils/Status.h"

namespace milvus::dog_segment {

class AckResponder {
 public:
    void AddSegment(int64_t seg_begin, int64_t seg_end) {
        std::lock_guard lck(mutex_);
        acks_.count(seg_begin);
    }

    void alter(int64_t endpoint) {
        if(acks_.count(endpoint)) {
            
        }
    }
 private:
    std::shared_mutex mutex_;
    std::set<int64_t> acks_ = {0};
};

int
TestABI() {
    return 42;
}

struct ColumnBasedDataChunk {
    std::vector<std::vector<float>> entity_vecs;
    static ColumnBasedDataChunk
    from(const DogDataChunk& source, const Schema& schema) {
        ColumnBasedDataChunk dest;
        auto count = source.count;
        auto raw_data = reinterpret_cast<const char*>(source.raw_data);
        auto align = source.sizeof_per_row;
        for (auto& field : schema) {
            auto len = field.get_sizeof();
            assert(len % sizeof(float) == 0);
            std::vector<float> new_col(len * count / sizeof(float));
            for (int64_t i = 0; i < count; ++i) {
                memcpy(new_col.data() + i * len / sizeof(float), raw_data + i * align, len);
            }
            dest.entity_vecs.push_back(std::move(new_col));
            // offset the raw_data
            raw_data += len / sizeof(float);
        }
        return dest;
    }
};

class SegmentNaive : public SegmentBase {
 public:
    virtual ~SegmentNaive() = default;
    // SegmentBase(std::shared_ptr<FieldsInfo> collection);

    // TODO: originally, id should be put into data_chunk
    // TODO: Is it ok to put them the other side?
    Status
    Insert(int64_t size, const uint64_t* primary_keys, const Timestamp* timestamps,
           const DogDataChunk& values) override;

    // TODO: add id into delete log, possibly bitmap
    Status
    Delete(int64_t size, const uint64_t* primary_keys, const Timestamp* timestamps) override;

    // query contains metadata of
    Status
    Query(const query::QueryPtr& query, Timestamp timestamp, QueryResult& results) override;

    // stop receive insert requests
    // will move data to immutable vector or something
    Status
    Close() override;

    // BuildIndex With Paramaters, must with Frozen State
    // NOTE: index_params contains serveral policies for several index
    Status
    UpdateIndex() override {

        throw std::runtime_error("not implemented");
    }

    // Status AddIndex(const std::string& name, IndexingConfig) override {
    //     assert()
    // }

    Status
    DropRawData(std::string_view field_name) override {
        // TODO: NO-OP
        return Status::OK();
    }

    Status
    LoadRawData(std::string_view field_name, const char* blob, int64_t blob_size) override {
        // TODO: NO-OP
        return Status::OK();
    }

 private:
    struct MutableRecord {
        tbb::concurrent_vector<uint64_t> uids_;
        tbb::concurrent_vector<Timestamp> timestamps_;
        std::vector<tbb::concurrent_vector<float>> entity_vecs_;
        MutableRecord(int entity_size) : entity_vecs_(entity_size) {
        }
    };

    struct ImmutableRecord {
        std::vector<uint64_t> uids_;
        std::vector<Timestamp> timestamps_;
        std::vector<std::vector<float>> entity_vecs_;
        ImmutableRecord(int entity_size) : entity_vecs_(entity_size) {
        }
    };

    template <typename RecordType>
    Status
    QueryImpl(const RecordType& record, const query::QueryPtr& query, Timestamp timestamp, QueryResult& results);

    std::shared_ptr<MutableRecord>
    GetMutable() {
        if (ready_immutable_) {
            return nullptr;
        }
        std::shared_lock lck(mutex_);
        return record_mutable_;
    }

 public:
    ssize_t
    get_row_count() const override {
        return ack_count_.load(std::memory_order_relaxed);
    }

    SegmentState
    get_state() const override {
        return state_.load(std::memory_order_relaxed);
    }

    ssize_t
    get_deleted_count() const override {
        return 0;
    }

 public:
    friend std::shared_ptr<SegmentBase>
    CreateSegment(SchemaPtr schema, IndexMetaPtr index_meta);

 private:
    SchemaPtr schema_;
    std::shared_mutex mutex_;
    std::atomic<SegmentState> state_ = SegmentState::Open;

    // we should fuck them as a struct
    std::atomic<int64_t> ack_count_ = 0;

    tbb::concurrent_unordered_map<uint64_t, int> internal_indexes_;

    std::shared_ptr<MutableRecord> record_mutable_;

    // to determined that if immutable data if available
    std::atomic<bool> ready_immutable_ = false;
    std::shared_ptr<ImmutableRecord> record_immutable_ = nullptr;


    IndexMetaPtr index_meta_;
    std::unordered_map<int, knowhere::VecIndexPtr> vec_indexings_;

    // TODO: scalar indexing
    // std::unordered_map<int, knowhere::IndexPtr> scalar_indexings_;

    tbb::concurrent_unordered_multimap<int, Timestamp> delete_logs_;
};

std::shared_ptr<SegmentBase>
CreateSegment(SchemaPtr schema, IndexMetaPtr index_meta) {
    auto segment = std::make_shared<SegmentNaive>();
    segment->schema_ = schema;
    segment->index_meta_ = index_meta;
    segment->record_mutable_ = std::make_shared<SegmentNaive::MutableRecord>(schema->size());

    return segment;
}

Status
SegmentNaive::Insert(int64_t size, const uint64_t* primary_keys, const Timestamp* timestamps,
                     const DogDataChunk& row_values) {
    const auto& schema = *schema_;
    auto record_ptr = GetMutable();
    assert(record_ptr);
    auto& record = *record_ptr;
    auto data_chunk = ColumnBasedDataChunk::from(row_values, schema);

    // TODO: use shared_lock for better concurrency
    std::lock_guard lck(mutex_);
    assert(state_ == SegmentState::Open);
    auto ack_id = ack_count_.load();
    record.uids_.grow_by(primary_keys, primary_keys + size);
    for (int64_t i = 0; i < size; ++i) {
        auto key = primary_keys[i];
        auto internal_index = i + ack_id;
        internal_indexes_[key] = internal_index;
    }
    record.timestamps_.grow_by(timestamps, timestamps + size);
    for (int fid = 0; fid < schema.size(); ++fid) {
        auto field = schema[fid];
        auto total_len = field.get_sizeof() * size / sizeof(float);
        auto source_vec = data_chunk.entity_vecs[fid];
        record.entity_vecs_[fid].grow_by(source_vec.data(), source_vec.data() + total_len);
    }

    // finish insert
    ack_count_ += size;
    return Status::OK();
}

Status
SegmentNaive::Delete(int64_t size, const uint64_t* primary_keys, const Timestamp* timestamps) {
    for (int i = 0; i < size; ++i) {
        auto key = primary_keys[i];
        auto time = timestamps[i];
        delete_logs_.insert(std::make_pair(key, time));
    }
    return Status::OK();
}

// TODO: remove mock

template <typename RecordType>
Status
SegmentNaive::QueryImpl(const RecordType& record, const query::QueryPtr& query, Timestamp timestamp,
                        QueryResult& result) {
    auto ack_count = ack_count_.load();
    assert(query == nullptr);
    assert(schema_->size() >= 1);
    const auto& field = schema_->operator[](0);
    assert(field.get_data_type() == DataType::VECTOR_FLOAT);
    assert(field.get_name() == "fakevec");
    auto dim = field.get_dim();
    // assume query vector is [0, 0, ..., 0]
    std::vector<float> query_vector(dim, 0);
    auto& target_vec = record.entity_vecs_[0];
    int current_index = -1;
    float min_diff = std::numeric_limits<float>::max();
    for (int index = 0; index < ack_count; ++index) {
        float diff = 0;
        int offset = index * dim;
        for (auto d = 0; d < dim; ++d) {
            auto v = target_vec[offset + d] - query_vector[d];
            diff += v * v;
        }
        if (diff < min_diff) {
            min_diff = diff;
            current_index = index;
        }
    }
    QueryResult query_result;
    query_result.row_num_ = 1;
    query_result.result_distances_.push_back(min_diff);
    query_result.result_ids_.push_back(record.uids_[current_index]);
    query_result.data_chunk_ = nullptr;
    result = std::move(query_result);
    return Status::OK();
}

Status
SegmentNaive::Query(const query::QueryPtr& query, Timestamp timestamp, QueryResult& result) {
    auto record_ptr = GetMutable();
    if (record_ptr) {
        return QueryImpl(*record_ptr, query, timestamp, result);
    } else {
        assert(ready_immutable_);
        return QueryImpl(*record_immutable_, query, timestamp, result);
    }
}

Status
SegmentNaive::Close() {
    auto src_record = GetMutable();
    assert(src_record);

    auto dst_record = std::make_shared<ImmutableRecord>(schema_->size());

    auto data_move = [](auto& dst_vec, const auto& src_vec) {
        assert(dst_vec.size() == 0);
        dst_vec.insert(dst_vec.begin(), src_vec.begin(), src_vec.end());
    };
    data_move(dst_record->uids_, src_record->uids_);
    data_move(dst_record->timestamps_, src_record->uids_);

    assert(src_record->entity_vecs_.size() == schema_->size());
    assert(dst_record->entity_vecs_.size() == schema_->size());
    for(int i = 0; i < schema_->size(); ++i) {
        data_move(dst_record->entity_vecs_[i], src_record->entity_vecs_[i]);
    }
    bool ready_old = false;
    record_immutable_ = dst_record;
    ready_immutable_.compare_exchange_strong(ready_old, true);
    if(ready_old) {
        throw std::logic_error("Close may be called twice, with potential race condition");
    }

    return Status::OK();
}

}  // namespace milvus::dog_segment
