 aux_source_directory(. CORE_LIB_DIR)
 
 add_library(CoreLib STATIC ${CORE_LIB_DIR})
 
 #使用c++17 filesystem时，需要额外指定stdc++fs链接库，因为c++17还未完全支持
 target_link_libraries(CoreLib cctz ${Poco_Foundation_LIBRARY} stdc++fs)