
# 使用Server.cc构建DataBaseServer库
add_library(DataBaseServer STATIC Server.cc)
#DataBaseServer库需要依赖DaemonLib库，因此需要链接
link_libraries(DataBaseServer DaemonLib)

#生成可执行文件db
add_executable(db main.cc)

#可执行文件的生成需要链接以下库
target_link_libraries(db PocoUtil CommonUtil DataBaseServer)
