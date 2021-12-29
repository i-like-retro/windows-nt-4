cmake_minimum_required(VERSION 3.0)
project(newapi)

# NOTE: this code is intended to be compiled with MSVC 6

add_definitions(
    -D_WIN32_WINNT=0x400
    -DWINVER=0x400
    )

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    )

if(CMAKE_COMPILER_IS_GNUCC)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-ident -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -march=i586")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--build-id=none -Wl,-s")
endif()

if(MSVC)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2 /Ot /Ob2 /Ox /Oi /Og /Oy")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /release /opt:ref /opt:icf /opt:nowin98")
endif()

add_library(ntcompat SHARED
    hardlink.cpp
    secure.c
    thread.c
    wspiapi.c
    )

target_link_libraries(ntcompat "${CMAKE_CURRENT_SOURCE_DIR}/ntdll.lib" ws2_32)
set_target_properties(ntcompat PROPERTIES PREFIX "")
