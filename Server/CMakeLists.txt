cmake_minimum_required(VERSION 3.3)
project(Server)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(SOURCE_FILES src/main.c src/server.c src/server.h src/communication.c src/communication.h)
add_executable(Server ${SOURCE_FILES})