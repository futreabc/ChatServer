#将指定目录下的所有源文件加入变量SRC_LIST中
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)
#生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})
#链接库文件
target_link_libraries(ChatServer pthread muduo_net muduo_base jsoncpp mysqlclient hiredis) 
 