cmake_minimum_required(VERSION 3.0.0)
project(ShadowMaps)

find_package(OpenGL REQUIRED)

set(QT5_ROOT "QT5-NOT_FOUND" CACHE PATH "QT5-NOT_FOUND")
if (EXISTS ${QT5_ROOT})
  message(STATUS ${QT5_ROOT})
  set(CMAKE_PREFIX_PATH "${QT5_ROOT}")
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5OpenGL REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory(sources)
