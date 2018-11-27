#生成可执行文件db
list(APPEND SERVER_SOURCES main.cc)
list(APPEND SERVER_SOURCES Server.cc)
list(APPEND SERVER_SOURCES HttpHandler.cc)
list(APPEND SERVER_SOURCES RootRequestHandler.cc)
list(APPEND SERVER_SOURCES PingRequestHandler.cc)
list(APPEND SERVER_SOURCES NotFoundHandler.cc)

add_executable(db main.cc ${SERVER_SOURCES})
#可执行文件的生成需要链接以下库
list(APPEND FNIAL_TARGET_DEP_LIBS ${Poco_Util_LIBRARY})
list(APPEND FNIAL_TARGET_DEP_LIBS ExceptionLib)
list(APPEND FNIAL_TARGET_DEP_LIBS CommonUtil)
list(APPEND FNIAL_TARGET_DEP_LIBS DaemonLib)
list(APPEND FNIAL_TARGET_DEP_LIBS AggregationFunctionLib)
list(APPEND FNIAL_TARGET_DEP_LIBS CommonFunctionLib)
list(APPEND FNIAL_TARGET_DEP_LIBS InterpreterLib)

target_link_libraries(db ${FNIAL_TARGET_DEP_LIBS})
