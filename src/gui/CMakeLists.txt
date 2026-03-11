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

# build the real binary with a different output name so we can wrap it in a launcher
add_executable(cli_drc_gui_real ${GUI_SOURCES} ${GUI_HEADERS})

# include paths for the real binary
target_include_directories(cli_drc_gui_real PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# link against engine and Qt
target_link_libraries(cli_drc_gui_real PRIVATE drc_engine Qt5::Widgets)

set_target_properties(cli_drc_gui_real PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "cli_drc_gui.real"
)

# generate a launcher script named cli_drc_gui that unsets unwanted env vars
# use configure_file to avoid quoting issues
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/launcher.sh.in
    ${CMAKE_BINARY_DIR}/bin/cli_drc_gui
    @ONLY
)
# ensure the script is executable immediately and after reconfigure
file(CHMOD ${CMAKE_BINARY_DIR}/bin/cli_drc_gui PERMISSIONS
     OWNER_READ OWNER_WRITE OWNER_EXECUTE
     GROUP_READ GROUP_EXECUTE
     WORLD_READ WORLD_EXECUTE)
# also update permission every time the real binary is built
add_custom_command(TARGET cli_drc_gui_real POST_BUILD
    COMMAND chmod +x ${CMAKE_BINARY_DIR}/bin/cli_drc_gui
    COMMENT "Making launcher script executable"
)
