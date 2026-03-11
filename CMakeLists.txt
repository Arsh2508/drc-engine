cmake_minimum_required(VERSION 3.16)

project(cli_drc VERSION 1.0 DESCRIPTION "Design Rule Checking System" LANGUAGES CXX)

# use modern C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Force linking against system libraries, not Snap
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags -Wl,-rpath,/usr/lib/x86_64-linux-gnu:/lib/x86_64-linux-gnu")

# Global output directory for all executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# fetch nlohmann json (header-only)
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# header-only interface library representing the engine
add_library(drc_engine INTERFACE)
target_include_directories(drc_engine INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(drc_engine INTERFACE nlohmann_json::nlohmann_json)

# ---- CLI executables ----
add_executable(cli_drc src/app/main.cpp)
target_link_libraries(cli_drc PRIVATE drc_engine)

add_executable(performance_comparison src/app/performance_comparison.cpp)
target_link_libraries(performance_comparison PRIVATE drc_engine)

add_executable(rule_tests src/tests/rule_tests.cpp)
target_link_libraries(rule_tests PRIVATE drc_engine)

# ---- optional GUI ----
option(BUILD_GUI "Build the Qt-based GUI" ON)
if(BUILD_GUI)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    add_subdirectory(src/gui)
endif()

# convenience target to print module summary
add_custom_target(show_structure
    COMMAND ${CMAKE_COMMAND} -E echo "Project modules: engine (headers), CLI, tests, optional Qt GUI"
)
