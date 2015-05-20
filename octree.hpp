#ifndef OCTREE_HPP
#define OCTREE_HPP
#include "octree.hpp"

#include <vector>

// #include <GL/glew.h> // GLEW, for sim.hpp

// // GLM
// #define GLM_FORCE_RADIANS
// #define GLM_SWIZZLE // for <glm::vec4>.xyz
// #include <glm/glm.hpp>
// #include <glm/gtc/constants.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// using namespace glm;

#include "gfx.hpp"
#include "sim.hpp"
// #include "controls.hpp"

// AABB bounding box
struct Bounds
{
  glm::vec3 center, half_width;
};
typedef struct Bounds Bounds;

class Octree
{
public:
    Bounds bbox;
    bool interior;
    glm::vec4 com; // center of mass (position, size)
    std::vector<Particle*> bods;

    Octree* children[8];

    Octree();
    Octree(const glm::vec3& center, const glm::vec3& half_width);
    ~Octree();

    void Init();
    void clear();

    bool isInBounds(const glm::vec3& point);

    // void calcForces(Particle* p, )

    void insert(Particle* p, int maxDepth = 0, int depth = 0);
    int getChildIndex(const glm::vec3& point);
    int getStats(int& nodes, int& leafs);
    
    void calculateMainBounds();
};



Octree* generateOctree();


#endif