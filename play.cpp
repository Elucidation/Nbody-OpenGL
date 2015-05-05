// Standard Headers
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include <GL/glew.h> // GLEW
#include <GLFW/glfw3.h> // GLFW
GLFWwindow* window; // Needed by controls.cpp

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

#include "loadShader.hpp" // Shader
#include "texture.hpp" // TextureIDs
#include "controls.hpp" // Controls
#include "sim.cpp" // Sim (contains N count etc.)
#include "gfx.cpp" // Graphics call

int main(int argc, char const *argv[])
{
    initGfx();

    double lastTime = glfwGetTime();
    double lastFPStime = glfwGetTime();
    int nbFrames = 0;
    long ParticlesCount = 0;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Determine dt
        double currentTime = glfwGetTime();
        double dt = currentTime - lastTime;
        lastTime = currentTime;

        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs();
        glm::mat4 ViewMatrix = getViewMatrix();
        // We will need the camera's position in order to sort the particles
        // w.r.t the camera's distance.
        // There should be a getCameraPosition() function in common/controls.cpp, 
        // but this works too.
        glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

        ///////////////////////////////////
        // SIMULATION

        currentTime = glfwGetTime();

        // Age and kill particles
        ParticlesCount = ageKillResetParticles(dt); // Also zeros accelerations before calculateAccelerations()

        // Create new particles based on how many left
        createNewParticles(ParticlesCount, dt);

        // Simulate all particles
        // unsigned long numForceCalcs = simulateEuler(dt);
        unsigned long numForceCalcs = simulateLeapfrog(dt);

        double simDelta = glfwGetTime() - currentTime;

        ///////////////////////////////////
        // SORT UPDATE
        currentTime = glfwGetTime();
        SortParticles(); // slow for > 100k particles, needed to look nice, but ignore for now
        double sortDelta = glfwGetTime() - currentTime;

        ///////////////////////////////////
        // GRAPHICS UPDATE
        currentTime = glfwGetTime();

        updatePositionColorBuffer(g_particle_position_size_data, g_particle_color_data, CameraPosition);

        updateGfx(programID, g_particle_position_size_data, g_particle_color_data,
                  &CameraPosition, ParticlesCount);

        double gfxDelta = glfwGetTime() - currentTime;

        // FPS Counter
        nbFrames++;
        if ( glfwGetTime() - lastFPStime >= 1.0 ){ // If last prinf() was more than 1 sec ago
            // printf and reset timer
            printf("%04.4f ms/frame \t (N: %.2fK, CALC: %10ld) \t|\t [%03.2f ms/sim, %03.2f ms/sort, %03.2f ms/gfx]\n", 1000.0/double(nbFrames), (double)(ParticlesCount)/1000.0, numForceCalcs, simDelta*1000.0, sortDelta*1000.0, gfxDelta*1000.0);
            nbFrames = 0;
            lastFPStime += 1.0f;
        }
    }

    // Wrap up
    glfwDestroyWindow(window);

    delete[] g_particle_position_size_data;
    delete[] g_particle_color_data;

    // Cleanup VBO and shader
    glDeleteBuffers(1, &particles_color_buffer);
    glDeleteBuffers(1, &particles_position_buffer);
    glDeleteBuffers(1, &billboard_vertex_buffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}


