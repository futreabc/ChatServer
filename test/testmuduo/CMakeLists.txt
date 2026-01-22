cmake_minimum_required(VERSION 3.0)
#工程的名字
project(main)
#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
#设置可执行文件的生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#配置头文件搜索路径
#include_directories()
#配置库文件搜索路径
#link_libraries()
#设置需要编译的源文件
set(SRC_LIST ./testmudo.cpp)
# 覆盖 SRC_LIST 为当前目录下所有源文件
#aux_source_directory(. SRC_LIST)
#生成可执行文件，由SRC_LIST而来
add_executable(server ${SRC_LIST})
#连接库文件
target_link_libraries(server muduo_net muduo_base pthread)
