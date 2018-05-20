aux_source_directory(. COMMON_UTIL_DIR)

add_library(CommonUtil STATIC ${COMMON_UTIL_DIR})

target_link_libraries(CommonUtil ${Poco_Net_LIBRARY})

add_subdirectory(test)
