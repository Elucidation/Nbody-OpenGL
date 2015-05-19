#ifndef SIM_HPP
#define SIM_HPP
// Contains functions for simulating particles

#include <algorithm> // for length2

#include <GL/glew.h> // GLEW
#include <GLFW/glfw3.h> // GLFW

// GLM
#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE // for <glm::vec4>.xyz
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/component_wise.hpp>
using namespace glm;

// Forward Declarations
class Octree;
struct Bounds;

// CPU representation of a particle
struct Particle{
    glm::vec3 pos, vel, acc;
    unsigned char r,g,b,a; // Color
    float size;
    float life; // Remaining life of the particle. if <0 : dead and unused.
    float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

    bool operator<(const Particle& that) const {
        // Sort in reverse order : far particles drawn first.
        return this->cameradistance > that.cameradistance;
    }
};

typedef struct Particle Particle;

extern const int MaxParticles;
extern Particle ParticlesContainer[];


void SortParticles();

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle();

void updatePositionsVelocities(float delta);
void updatePositions(float delta);
void updateVelocities(float delta);
void updatePositionColorBuffer(GLfloat* g_particle_position_size_data, 
    GLubyte* g_particle_color_data, 
    glm::vec3 CameraPosition);

// Needs acceleration vectors to be zeroed before running
// ^ Normally called automatically by ageKillResetParticles
unsigned long calc_all_acc_brute();

unsigned long calc_all_acc_barnes_hut(Octree* rootOctree);
unsigned long acc_barnes_hut(Particle& body, Octree* root);
void calc_acc(Particle& body, const glm::vec4& com);
void calc_acc(Particle& body, const Particle& other);

// Also resets accelerations
unsigned long ageKillResetParticles(double delta);
void createNewParticles(unsigned long ParticlesCount, double delta);

// Euler integration
unsigned long simulateEuler(double dt);

// Leapfrog integration
unsigned long simulateLeapfrog(Octree* oct, double dt);

#endif