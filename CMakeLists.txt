cmake_minimum_required(VERSION 3.16)
project(CodeRunner VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 or Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
else()
    message(STATUS "Using Qt6")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/codeeditor.cpp
    src/filemanager.cpp
    src/compiler.cpp
    src/debugger.cpp
    src/compileroutputpanel.cpp
    src/errorparser.cpp
    src/projectconfig.cpp
    src/compilersettingsdialog.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/codeeditor.h
    src/filemanager.h
    src/compiler.h
    src/debugger.h
    src/compileroutputpanel.h
    src/errorparser.h
    src/projectconfig.h
    src/compilersettingsdialog.h
)

# Resource files
set(RESOURCES
    src/resources.qrc
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Link Qt libraries
if(Qt6_FOUND)
    target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
    )
else()
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
    )
endif()

# Platform-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE ON)
endif()

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
