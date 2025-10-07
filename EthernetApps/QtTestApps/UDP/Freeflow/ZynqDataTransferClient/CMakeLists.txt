cmake_minimum_required(VERSION 3.16)

project(ZynqDataTransferClient VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to find Qt6 first, fallback to Qt5
find_package(Qt6 QUIET COMPONENTS Core Widgets Network)
if(Qt6_FOUND)
    qt6_standard_project_setup()
    set(QT_VERSION 6)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
    qt5_standard_project_setup()
    set(QT_VERSION 5)
endif()

set(SOURCES
    main.cpp
    mainwindow.cpp
)

set(HEADERS
    mainwindow.h
)

if(QT_VERSION EQUAL 6)
    qt6_add_executable(ZynqDataTransferClient
        ${SOURCES}
        ${HEADERS}
    )
    
    target_link_libraries(ZynqDataTransferClient
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
    )
    
    set_target_properties(ZynqDataTransferClient PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )
else()
    qt5_add_executable(ZynqDataTransferClient
        ${SOURCES}
        ${HEADERS}
    )
    
    target_link_libraries(ZynqDataTransferClient
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
    )
    
    set_target_properties(ZynqDataTransferClient PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )
endif()
