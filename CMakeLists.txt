cmake_minimum_required(VERSION 3.0)

project(tetris VERSION 0.1 LANGUAGES CXX)

set_target_properties(${TARGET} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
add_compile_definitions(SFML_STATIC)

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
else()
    target_link_libraries(tetris pthread X11 sfml-window sfml-graphics sfml-system)
endif(WIN32)
