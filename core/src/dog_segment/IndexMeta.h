#pragma once

#include "SegmentDefs.h"
#include "knowhere/index/IndexType.h"
#include <shared_mutex>

namespace milvus::dog_segment {
// TODO: this is
class IndexMeta {
 public:
    IndexMeta(SchemaPtr schema): schema_(schema) {}
    using IndexType = knowhere::IndexType;
    using IndexMode = knowhere::IndexMode;
    using IndexConfig = knowhere::Config;

    struct Entry {
        uint64_t entry_uid;
        std::string index_name;
        IndexType type;
        IndexMode mode;
        IndexConfig config;
    };

    Status
    AddEntry(const std::string& index_name, const std::string& field_name, IndexType type, IndexMode mode, IndexConfig config);

    Status
    DropEntry(std::string_view index_name);

 private:
    bool VerifyEntry(Entry& entry);

 private:
    std::shared_mutex mutex_;
    std::atomic<uint64_t> uid_source_ = 0;
    SchemaPtr schema_;
    std::map<std::string, Entry> entries_; // index_name => Entry
    std::map<std::string, std::string> lookups_; // field_name => index_name
};

using IndexMetaPtr = std::shared_ptr<IndexMeta>;

}
