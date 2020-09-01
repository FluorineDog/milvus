#include "IndexMeta.h"
namespace milvus::dog_segment {

Status
IndexMeta::AddEntry(const std::string& index_name, const std::string& field_name, IndexType type, IndexMode mode,
                    IndexConfig config) {
    Entry entry{
        uid_source_++,
        field_name,
        type,
        mode,
        std::move(config)
    };
    std::shared_lock lck(mutex_);
    if (entries_.count(index_name)) {
        throw std::invalid_argument("duplicate index_name");
    }

    // TODO: enable multiple index for fields
    return Status::OK();
}

Status
IndexMeta::DropEntry(std::string_view index_name) {
    return Status::OK();
}

}  // namespace milvus::dog_segment
