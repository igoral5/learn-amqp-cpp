project(learn-amqp-cpp CXX)

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(consumer consumer.cpp)

add_executable(producer producer.cpp asiohandler.h asiohandler.cpp)
target_link_libraries(producer boost_system pthread amqpcpp)
