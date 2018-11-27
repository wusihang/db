 aux_source_directory(. IO_LIB_DIR)
 
 add_library(IOLib STATIC ${IO_LIB_DIR})
 
 target_link_libraries(IOLib ${Poco_Foundation_LIBRARY} CommonLib double-conversion cityhash)