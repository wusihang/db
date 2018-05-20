add_executable(testParser testParser.cc)
add_executable(testUseQuery testUseQuery.cc)

target_link_libraries(testParser ParserLib)
target_link_libraries(testUseQuery ParserLib)