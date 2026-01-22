#包含源文件(.cpp)
aux_source_directory(. SRC_LIST)
#生成指定的可执行文件
add_executable(ChatClient ${SRC_LIST})
#包含所需要的库文件
target_link_libraries(ChatClient pthread jsoncpp)