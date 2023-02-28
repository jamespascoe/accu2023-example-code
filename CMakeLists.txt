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

    add_executable(http_client_sync http_client_sync.cpp)
    target_link_libraries(http_client_sync stdc++_libbacktrace)

    add_executable(http_client_awaitable http_client_awaitable.cpp)

    add_executable(http_client_coro http_client_coro.cpp)
    target_link_libraries(http_client_coro ${Boost_LIBRARIES})

    add_executable(http_server_sync http_server_sync.cpp)
    add_executable(http_server_async http_server_async.cpp)
    add_executable(http_server_awaitable http_server_awaitable.cpp)

    add_executable(http_server_coro http_server_coro.cpp)
    target_link_libraries(http_server_coro ${Boost_LIBRARIES})
endif()
