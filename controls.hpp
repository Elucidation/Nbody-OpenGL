#ifndef CONTROLS_HPP
#define CONTROLS_HPP

// #include "gfx.hpp"

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

// // GLM
// #define GLM_FORCE_RADIANS
// #define GLM_SWIZZLE // for <glm::vec4>.xyz
// #include <glm/glm.hpp> 
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtx/norm.hpp>
// #include <glm/gtx/string_cast.hpp>
// using namespace glm;

extern bool doRun;
extern bool showOct;

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif