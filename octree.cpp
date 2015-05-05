// Used for oct-tree decomposition & Barnes-Hut algorithm in nbody sim

// AABB bounding box
struct Bounds
{
  glm::vec3 min, max;
};

Bounds calculateBounds()
{
    Bounds b;
    b.min = glm::vec3(10000,10000,10000);
    b.max = glm::vec3(-100000,-100000,-100000);
    for(int i=0; i<MaxParticles; i++)
    {
        Particle& p = ParticlesContainer[i]; // shortcut        
        if(p.life > 0.0f)
        {
            b.min = min(b.min, p.pos);
            b.max = max(b.max, p.pos);
        }
    }
    return b;
}