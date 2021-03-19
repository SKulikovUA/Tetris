cmake_minimum_required(VERSION 3.0)

project(tetris VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set_target_properties(${TARGET} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
add_compile_definitions(SFML_STATIC)

find_package(PkgConfig REQUIRED)
pkg_search_module(SFML REQUIRED SFML-graphics)

include_directories(${SFML_INCLUDE_DIRS})
link_directories(${SFML_LIBRARY_DIRS})

if(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
    include_directories( ${PROJECT_SOURCE_DIR}/externals/SFML/include )
    link_directories( ${PROJECT_SOURCE_DIR}/externals/SFML/lib )
endif(WIN32)
add_executable(tetris main.cpp CTetris.cpp)
if(WIN32)
    if(DEBUG)
        target_link_libraries(tetris opengl32 winmm freetype sfml-window-s-d sfml-main-d sfml-graphics-s-d sfml-system-s-d)
    else()
        target_link_libraries(tetris opengl32 winmm freetype sfml-window-s sfml-main sfml-graphics-s sfml-system-s)
    endif(DEBUG)
endif(WIN32)

if(UNIX_AND_NOT_APPLE)
    target_link_libraries(tetris pthread X11 sfml-window sfml-graphics sfml-system)
endif(UNIX_AND_NOT_APPLE)

if(APPLE)
    target_link_libraries(tetris pthread sfml-window sfml-graphics sfml-system)
endif(APPLE)

