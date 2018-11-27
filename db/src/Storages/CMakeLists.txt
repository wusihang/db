aux_source_directory(. STORAGE_DIR)
aux_source_directory(MergeTree STORAGE_DIR_MERGE_TREE)

add_library(StorageLib STATIC ${STORAGE_DIR} ${STORAGE_DIR_MERGE_TREE})

target_link_libraries(StorageLib CommonLib StreamsLib CoreLib InterpreterLib)
