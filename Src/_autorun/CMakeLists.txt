cmake_minimum_required(VERSION 3.5)
project(Autorun)

if(MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-ident -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -march=i386")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--build-id=none -Wl,-s -static-libgcc")
endif()

add_executable(autorun WIN32
    autorun.c
    nuklear.h
    nuklgdi.h
    )

set_target_properties(autorun
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../CD"
    OUTPUT_NAME "AUTORUN" PREFIX "" SUFFIX ".EXE"
    )
