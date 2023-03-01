cmake_minimum_required(VERSION 3.14)

project (CoWeb)

set (CMAKE_CXX_STANDARD 23)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Boost COMPONENTS coroutine)

if(Boost_FOUND)
    link_libraries(Threads::Threads)

    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(sync_http_server sync_http_server.cpp)
    add_executable(async_http_server async_http_server.cpp)
    add_executable(coro_http_server coro_http_server.cpp)

    target_link_libraries(coro_http_server ${Boost_LIBRARIES})
endif()
