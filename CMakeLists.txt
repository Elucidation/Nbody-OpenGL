cmake_minimum_required (VERSION 2.6)
project ( PLAY )


## WAY 1
# find_package(OpenGL REQUIRED)
# if(NOT OPENGL_FOUND)
#     message("ERROR: OpenGL not found")
# endif(NOT OPENGL_FOUND)
# set(GL_LIBRARY GL GLU X11)
#
# add_executable(play play.cpp)
#
# linking "glfw" and not "glfw3" 
# assumes that glfw was built with BUILD_SHARED_LIBS to ON
# target_link_libraries(play glfw ${GL_LIBRARY})

## WAY on glfw.org
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

add_executable(play play.cpp)

target_link_libraries(play ${GLFW_LIBRARIES})