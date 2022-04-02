cmake_minimum_required(VERSION 3.21)
project(msdp-c)

set(CMAKE_CXX_STANDARD 14)

set(PROJECT_SOURCE_DIR protocol)

include_directories(protocol)

add_executable(msdp-c main.c
        pi-client.c
        protocol/msdp-net.c
        protocol/msdp-client.c
        pi-client.h
        protocol/msdp-packet-utils.c
        protocol/msdp-packet-utils.h
        protocol/msdp-uid.h protocol/msdp-log.c protocol/msdp-log.h)

if(WIN32)
    target_link_libraries(msdp-c ws2_32)
endif()
