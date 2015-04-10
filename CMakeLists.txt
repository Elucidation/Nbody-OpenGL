
cmake_minimum_required (VERSION 2.6)
project ( PLAY )

# Make sure PkgConfig is available
find_package(PkgConfig REQUIRED)

# Add GLFW
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

# Add GLEW
find_package(GLEW REQUIRED)
if (NOT GLEW_FOUND)
  message("ERROR: GLEW not found")
endif()
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

# Add OpenGL
find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message("ERROR: OpenGL not found")
endif(NOT OPENGL_FOUND)
set(GL_LIBRARY GL GLU X11)

# Executables
add_executable(play play.cpp)

# Link libraries
target_link_libraries(play ${GLFW_LIBRARIES} ${GL_LIBRARY})

# Copy shaders over
configure_file(BasicVertexShader.vert BasicVertexShader.vert COPYONLY)
configure_file(BasicFragmentShader.frag BasicFragmentShader.frag COPYONLY)