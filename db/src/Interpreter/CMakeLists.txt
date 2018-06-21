aux_source_directory(. INTERPRETER_SOURCE_DIR)

add_library(InterpreterLib ${INTERPRETER_SOURCE_DIR})

target_link_libraries(InterpreterLib ParserLib DataBasesLib StorageLib DataTypeLib)