set(BUILD_TARGET "shadowmaps")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_DURRENT_DIR ON)

configure_file(${CMAKE_CURRENT_LIST_DIR}/common.h.in
               ${CMAKE_CURRENT_LIST_DIR}/common.h @ONLY)

set(SOURCES main.cpp maingui.h maingui.cpp
            openglviewer.h openglviewer.cpp
            arcballcontroller.h glutils.h valueslider.h
            common.h tiny_obj_loader.h)

file(GLOB SHADERS "shaders/*")

add_executable(${BUILD_TARGET} ${SOURCES} ${SHADERS})
qt5_use_modules(${BUILD_TARGET} Widgets OpenGL)

target_link_libraries(${BUILD_TARGET} ${OPENGL_LIBRARIES} ${QT_LIBRARIES})

source_group("Source Files" FILES ${SOURCES})
source_group("Shader Files" FILES ${SHADERS})
