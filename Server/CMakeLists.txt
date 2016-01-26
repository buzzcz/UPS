cmake_minimum_required(VERSION 3.3)
project(Server)

find_package(Threads)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lpthread -g")

set(SOURCE_FILES src/main.c src/server.c src/server.h src/communication.c src/communication.h src/game.c src/game.h src/structures.h src/list.c src/list.h)
add_executable(Server ${SOURCE_FILES})
target_link_libraries(Server ${CMAKE_THREAD_LIBS_INIT})
add_custom_command(TARGET Server POST_BUILD        # Adds a post-build event to MyTest
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/words.txt"      # <--this is in-file
        $<TARGET_FILE_DIR:Server>)                 # <--this is out-file path
