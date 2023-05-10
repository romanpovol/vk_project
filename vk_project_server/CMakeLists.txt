cmake_minimum_required(VERSION 3.14)

project(vk_project_server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_executable(vk_project_server
  main.cpp
  server.cpp
  server.h
)
target_link_libraries(vk_project_server Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(vk_project_server Qt${QT_VERSION_MAJOR}::Network)

include_directories(usr/include)
link_directories(usr/lib)
target_link_libraries(vk_project_server mysqlcppconn)
target_compile_features(vk_project_server PRIVATE cxx_range_for)

install(TARGETS vk_project_server
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
