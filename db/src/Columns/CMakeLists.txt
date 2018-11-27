 aux_source_directory(. COLUMNS_LIB_DIR)
 
 add_library(ColumnsLib STATIC ${COLUMNS_LIB_DIR})
 
 target_compile_options(ColumnsLib PRIVATE -Wno-narrowing)