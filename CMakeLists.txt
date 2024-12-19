cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME a-star)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif(CCACHE_FOUND)

include_directories(
	birb3d/engine/core/include
	birb3d/engine/rendering/include
	birb3d/engine/scenes/include
	birb3d/engine/widgets/include
)

add_subdirectory(birb3d)

file(GLOB GAME_SOURCES ./src/*.cpp)
add_executable(${PROJECT_NAME} ${GAME_SOURCES})
target_link_libraries(${PROJECT_NAME} birb)
target_include_directories(${PROJECT_NAME} PUBLIC ./include)

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ./)
