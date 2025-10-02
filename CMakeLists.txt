cmake_minimum_required(VERSION 3.16)
project(qtngraph LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt libraries
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Add your sources
set(SOURCES
    src/include/Canvas.cpp
    src/include/net/Interfaces.cpp
    src/include/nodes/IPDisplayNode.cpp
    src/include/nodes/RouterNode.cpp
)

set(HEADERS
    src/include/Canvas.hpp
    src/include/net/Interfaces.hpp
    src/include/nodes/IPDisplayNode.hpp
    src/include/nodes/RouterNode.hpp
)

# Create executable
add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Gui Qt6::Widgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
