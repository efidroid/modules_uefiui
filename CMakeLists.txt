cmake_minimum_required(VERSION 2.8)
project(uefiui C ASM)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -Wstrict-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -Wl,-gc-sections -Os -ggdb -fno-builtin")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")

include_directories(
    include
    include_posix
)

add_executable(main
    src/main.c

    src/maintest.c
    src/uui/fb.c
    src/uui/canvas.c
    src/uui/views/view.c
    src/uui/views/viewgroup.c
    src/uui/views/rect.c

    src/uui/layouts/linear.c
    src/uui/layouts/absolute.c
)
target_link_libraries(main SDL)
