aux_source_directory(. STORAGE_DIR)

add_library(StorageLib STATIC ${STORAGE_DIR})

target_link_libraries(StorageLib CommonLib)