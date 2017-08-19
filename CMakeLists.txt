cmake_minimum_required(VERSION 2.8)
project(uefiui C ASM)
set(CMAKE_BUILD_TYPE Debug)

find_package(BISON)
find_package(FLEX)

# use flex from PATH
set(FLEX_EXECUTABLE flex)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -Wstrict-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -Wl,-gc-sections -Os -ggdb -fno-builtin")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")

include_directories(
    include
    include_posix
)

add_definitions(
    -DYY_BARE_METAL
)

BISON_TARGET(uui_parser src/uui.y   ${CMAKE_CURRENT_BINARY_DIR}/uui_parser.c)
FLEX_TARGET(uui_lexer   src/uui.lex ${CMAKE_CURRENT_BINARY_DIR}/uui_lexer.c DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/uui_lexer.h)
ADD_FLEX_BISON_DEPENDENCY(uui_lexer uui_parser)

add_executable(main
    src/main.c

    src/maintest.c
    src/uui/fb.c
    src/uui/canvas.c
    src/uui/components.c
    src/uui/resources.c
    src/uui/views/view.c
    src/uui/views/viewgroup.c
    src/uui/views/rect.c

    src/uui/layouts/linear.c
    src/uui/layouts/absolute.c

    lib/hashmap.c
    lib/strhashmap.c

    ${BISON_uui_parser_OUTPUTS}
    ${FLEX_uui_lexer_OUTPUTS}
)
target_link_libraries(main SDL)
