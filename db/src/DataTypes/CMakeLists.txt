aux_source_directory(. DATA_TYPE_DIR)

add_library(DataTypeLib ${DATA_TYPE_DIR})

target_link_libraries(DataTypeLib ColumnsLib)