aux_source_directory(. STREAM_DIR)

add_library(StreamsLib STATIC ${STREAM_DIR})

target_link_libraries(StreamsLib)