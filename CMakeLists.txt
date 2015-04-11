
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
set(GL_LIBRARY GL GLU X11 Xi Xrandr Xcursor Xxf86vm pthread Xinerama Xft)

# Create Shader/Texture/Controls libraries
add_library(Shader loadShader.cpp loadShader.hpp)
add_library(Texture texture.cpp texture.hpp)
add_library(Controls controls.cpp controls.hpp)
set(SOURCES play.cpp loadShader.hpp texture.hpp controls.hpp)

# Executables
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(play ${SOURCES})


# Link libraries
set(COMMON Shader Texture Controls)
target_link_libraries(play ${GLFW_LIBRARIES} ${GL_LIBRARY} ${COMMON})

# Copy shaders over
configure_file(BasicVertexShader.vert BasicVertexShader.vert COPYONLY)
configure_file(BasicFragmentShader.frag BasicFragmentShader.frag COPYONLY)

# Copy textures over
# configure_file(uvtemplate.bmp uvtemplate.bmp COPYONLY)
# configure_file(uvtemplate.DDS uvtemplate.DDS COPYONLY)
configure_file(point.DDS point.DDS COPYONLY)
configure_file(point2.DDS point2.DDS COPYONLY)
configure_file(test.DDS test.DDS COPYONLY)