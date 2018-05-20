 aux_source_directory(. PARSER_LIB_DIR)
 
 add_library(ParserLib STATIC ${PARSER_LIB_DIR})
 
 
 target_link_libraries(ParserLib IOLib)
 
 add_subdirectory(test)