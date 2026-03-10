# GUI application build rules (included from top-level CMake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(GUI_SOURCES
    main.cpp
    MainWindow.cpp
    GraphicsView.cpp
    LayoutScene.cpp
)

set(GUI_HEADERS
    MainWindow.hpp
    GraphicsView.hpp
    LayoutScene.hpp
)

add_executable(cli_drc_gui ${GUI_SOURCES} ${GUI_HEADERS})

target_include_directories(cli_drc_gui PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# link against engine and Qt
target_link_libraries(cli_drc_gui PRIVATE drc_engine Qt5::Widgets)

set_target_properties(cli_drc_gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
