set(DOG_SEGMENT_FILES
        SegmentBasic.cpp
        )
# Third Party dablooms file
#aux_source_directory( ${MILVUS_THIRDPARTY_SRC}/dablooms  THIRDPARTY_DABLOOMS_FILES )
add_library( dog_segment STATIC )
target_sources( dog_segment PRIVATE
        ${DOG_SEGMENT_FILES}
        #${THIRDPARTY_DABLOOMS_FILES}
        )
#add_dependencies( segment sqlite mysqlpp )