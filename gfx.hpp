#ifndef GFX_HPP
#define GFX_HPP

#include <GL/glew.h> // GLEW
#include <GLFW/glfw3.h> // GLFW

// GLM
#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE // for <glm::vec4>.xyz
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
using namespace glm;


// #include "octree.hpp"
#include "controls.hpp"
#include "sim.hpp"

// Forward Declarations
class Octree;
struct Bounds;

extern GLfloat* g_particle_position_size_data;
extern GLubyte* g_particle_color_data;
extern GLFWwindow* window;


// Drawing
void drawOctCOM(Octree* root, const float size=1.f, const int depth=0, const int max_depth=10);
void drawParticles(GLubyte* g_particle_color_data, unsigned long ParticlesCount);
void drawOrigin();
void drawBounds(const Bounds& bbox, const GLubyte* color);
void drawMarker(const glm::vec3 point, const float half_width);
void drawBox(const glm::vec3 bmin, const glm::vec3 bmax, const GLubyte* color);
void drawTree(Octree* root, int depth=0, int maxDepth=8);
void colorParticles(Octree* oct);


// WIndow callbacks
void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Setup GFX
void setupWindowHints();
void initGlew();
GLFWwindow* setupGL();
void initGfx();
void gfxCleanup();

// Update GFX
void updateGfx(GLfloat* g_particle_position_size_data, 
    GLubyte* g_particle_color_data, unsigned long ParticlesCount, Octree* oct);

#endif