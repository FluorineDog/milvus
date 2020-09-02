#include "IndexMeta.h"
#include <mutex>
namespace milvus::dog_segment {

Status
IndexMeta::AddEntry(const std::string& index_name, const std::string& field_name, IndexType type, IndexMode mode,
                    IndexConfig config) {
    Entry entry{
        uid_source_++,
        index_name,
        field_name,
        type,
        mode,
        std::move(config)
    };
    if(!VerifyEntry(entry)) {
        throw std::invalid_argument("invalid entry");
    }
    std::lock_guard lck(mutex_);
    if (entries_.count(index_name)) {
        throw std::invalid_argument("duplicate index_name");
    }

    // TODO: enable multiple index for fields
    // TODO: now just use the most recent one
    lookups_[index_name] = index_name;

    return Status::OK();
}

Status
IndexMeta::DropEntry(std::string_view index_name) {
    std::lock_guard lck(mutex_);

    return Status::OK();
}

void IndexMeta::VerifyEntry(const Entry &entry) {
    auto is_mode_valid = std::set{IndexMode::MODE_CPU, IndexMode::MODE_GPU}.count(entry.mode);
    if(!is_mode_valid) {
        throw std::invalid_argument("invalid mode");
    }

    auto& schema = *schema_;
    auto& field_meta = schema[entry.index_name];
    if(field_meta.is_vector()) {

    }
    return true;
}

}  // namespace milvus::dog_segment
