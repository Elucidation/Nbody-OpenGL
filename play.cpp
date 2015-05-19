// Standard Headers
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include "controls.hpp" // Controls
#include "sim.hpp" // Sim (contains N count etc.)
#include "octree.hpp" // Octree
#include "gfx.hpp" // Graphics call (uses octree Bounds struct)

extern bool doRun;

int main(int argc, char const *argv[])
{
    printf("Controls: \n");
    printf("Space - Pause/Run\n");
    printf("T - Show/Hide Octree\n");
    printf("P - Enable/Disable Mouse Camera Control\n");
    
    initGfx();

    double lastTime = glfwGetTime();
    double lastFPStime = glfwGetTime();
    int nbFrames = 0;
    long ParticlesCount = 0;
    unsigned long numForceCalcs = 0;
    double simDelta;


    doRun = true; // Start with sim running

    Octree* oct = new Octree();

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
        if (doRun)
        {
            currentTime = glfwGetTime();

            // Age and kill particles
            ParticlesCount = ageKillResetParticles(dt); // Also zeros accelerations before calculateAccelerations()

            // Create new particles based on how many left
            createNewParticles(ParticlesCount, dt);

            // Simulate all particles
            // unsigned long numForceCalcs = simulateEuler(dt);
            numForceCalcs = simulateLeapfrog(dt);

            simDelta = glfwGetTime() - currentTime;
        } 
        else
        {
            simDelta = 0;
            numForceCalcs = 0;
        }

        ///////////////////////////////////
        // Octree Update
        currentTime = glfwGetTime();
        delete oct;
        oct = generateOctree();
        double octDelta = glfwGetTime() - currentTime;
        
        ///////////////////////////////////
        // SORT UPDATE
        currentTime = glfwGetTime();
        SortParticles(); // slow for > 100k particles, needed to look nice, but ignore for now
        double sortDelta = glfwGetTime() - currentTime;


        ///////////////////////////////////
        // GRAPHICS UPDATE
        currentTime = glfwGetTime();

        updatePositionColorBuffer(g_particle_position_size_data, g_particle_color_data, CameraPosition);

        updateGfx(g_particle_position_size_data, g_particle_color_data, ParticlesCount, oct);

        double gfxDelta = glfwGetTime() - currentTime;

        // FPS Counter
        nbFrames++;
        if ( glfwGetTime() - lastFPStime >= 1.0 ){ // If last prinf() was more than 1 sec ago
            // printf and reset timer
            printf("%04.4f ms/frame\t(N: %.2fK, CALC: %10ld) |\t [%03.2f ms/sim, %03.2f ms/oct, %03.2f ms/sort, %03.2f ms/gfx]\n", 
                1000.0/double(nbFrames), (double)(ParticlesCount)/1000.0, numForceCalcs, 
                simDelta*1000.0, octDelta*1000.0, sortDelta*1000.0, gfxDelta*1000.0);

            // int nodes=0, leafs=0;
            // oct->getStats(nodes, leafs);
            // printf("Octree | Nodes: %d, Leafs: %d\n", nodes, leafs);
            // Bounds b = calculateBounds();
            // printf("Bounds: %s - %s\n", to_string(b.min).c_str(), to_string(b.max).c_str());
            printf("Oct | COM: <%.2f, %.2f, %.2f> M:%g\n", oct->com.x, oct->com.y, oct->com.z, oct->com.w);
            nbFrames = 0;
            lastFPStime += 1.0f;
        }
    }

    // Wrap up
    gfxCleanup();

    exit(EXIT_SUCCESS);
}


