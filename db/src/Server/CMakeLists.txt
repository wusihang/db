#生成可执行文件db
add_executable(db main.cc Server.cc)

#可执行文件的生成需要链接以下库
target_link_libraries(db PocoUtil CommonUtil DaemonLib)
