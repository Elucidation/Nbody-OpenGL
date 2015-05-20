// Contains functions for simulating particles

#include "sim.hpp"

#include "octree.hpp"

// Physics constants
#define G 0.01f
#define ETA 0.01f

// Barnes Hut approximation constants
#define BARNES_HUT_RATIO_THRESHOLD 1.0
// Choosing something like 100 means  ~always choose root node, should be ~O(n) calculations
// Choosing 0 devolves to brute force O(n^2), with a worse constant than regular brute force

#define SPREAD 1.0f

const int MaxParticles = 10000;
Particle ParticlesContainer[MaxParticles];

const float NewParticleSpeed = 10000000.0f;
int LastUsedParticle = 0;


void SortParticles(){
    std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle(){

    for(int i=LastUsedParticle; i<MaxParticles; i++){
        if (ParticlesContainer[i].life < 0){
            LastUsedParticle = i;
            return i;
        }
    }

    for(int i=0; i<LastUsedParticle; i++){
        if (ParticlesContainer[i].life < 0){
            LastUsedParticle = i;
            return i;
        }
    }

    return 0; // All particles are taken, override the first one
}

void updatePositionsVelocities(float delta)
{
    for(int i=0; i<MaxParticles; i++)
    {
        Particle& p = ParticlesContainer[i]; // shortcut        
        if(p.life > 0.0f)
        {
            p.vel += p.acc * (float)delta;
            p.pos += p.vel * (float)delta;
        }
    }
}

void updatePositions(float delta)
{
    for(int i=0; i<MaxParticles; i++)
    {
        Particle& p = ParticlesContainer[i]; // shortcut        
        if(p.life > 0.0f)
        {
            p.pos += p.vel * (float)delta;
        }
    }
}

void updateVelocities(float delta)
{
    for(int i=0; i<MaxParticles; i++)
    {
        Particle& p = ParticlesContainer[i]; // shortcut        
        if(p.life > 0.0f)
        {
            p.vel += p.acc * (float)delta;
            p.vel *= 0.999; // damping
        }
    }
}

void updatePositionColorBuffer(GLfloat* g_particle_position_size_data, GLubyte* g_particle_color_data, glm::vec3 CameraPosition)
{
    unsigned int idx = 0;
    for(int i=0; i<MaxParticles; i++)
    {
        Particle& p = ParticlesContainer[i]; // shortcut        
        if(p.life > 0.0f)
        {
            p.cameradistance = glm::length2( p.pos - CameraPosition );
            //ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

            // Fill the GPU buffer
            g_particle_position_size_data[4*idx+0] = p.pos.x;
            g_particle_position_size_data[4*idx+1] = p.pos.y;
            g_particle_position_size_data[4*idx+2] = p.pos.z;
                                     
            g_particle_position_size_data[4*idx+3] = p.size;
                                     
            g_particle_color_data[4*idx+0] = p.r;
            g_particle_color_data[4*idx+1] = p.g;
            g_particle_color_data[4*idx+2] = p.b;
            g_particle_color_data[4*idx+3] = p.a;

            idx++;
        }
    }
}

// Needs acceleration vectors to be zeroed before running
// ^ Normally called automatically by ageKillResetParticles
unsigned long calc_all_acc_brute()
{
    // Simulate all particles
    unsigned int numForceCalcs = 0;
    for(int i=0; i<MaxParticles; i++){

        Particle& p = ParticlesContainer[i]; // shortcut

        // Ignore dead particles (they'll be reset later)
        if(p.life > 0.0f)
        {
            for (int j = i+1; j < MaxParticles; ++j)
            {
                Particle& q = ParticlesContainer[j]; // shortcut

                if (q.life > 0)
                {
                    glm::vec3 d = q.pos - p.pos; // distance
                    // glm::vec3 r2 = glm::length2(q.pos, p.pos); // length squared
                    float r2 = glm::dot(d,d); // distance squared
                    // float r = glm::sqrt(r2); // length ( != distance)
                    float r3 = r2 * glm::sqrt(r2);

                    // // Masses are from 0.01 to 0.06
                    float f_mag = (G*p.size*q.size) / (r3 + ETA);
                    glm::vec3 f = d * f_mag; // force vector

                    p.acc += f;
                    q.acc -= f;

                    numForceCalcs++;
                }
            }
        }
    }
    return numForceCalcs;
}

// calculate all accelerations for all particles
unsigned long calc_all_acc_barnes_hut(const Octree& rootOctree)
{
    unsigned int numForceCalcs = 0;
    for(int i=0; i<MaxParticles; i++)
    {
        numForceCalcs += acc_barnes_hut(ParticlesContainer[i], rootOctree);
    }
}

// Calculate accelerations on body using barnes hut algorithm on given octree
// in-place update particle's acceleration vector with it
unsigned long acc_barnes_hut(Particle& body, const Octree& root)
{
    
    unsigned int numForceCalcs = 0;
    // Check particle with root node
    // if node size / distance < theta threshold
    //   update body acceleration with node com and done
    // else
    //   recursively call acc_barnes_hut(body, child) for each child and done

    // Min width of node bounding box
    GLfloat s = glm::compMin(root.bbox.half_width) * 2.0f;

    // Distance from body to CoM of node
    GLfloat d = glm::distance(body.pos, root.com.xyz());

    // Test node size / distance threshold
    if (s/d < BARNES_HUT_RATIO_THRESHOLD)
    {
        // If sufficiently far away, approximate
        calc_acc(body, root.com);
        numForceCalcs++;
    }
    else if (!root.interior)
    {
        // Not far enough away, but is a leaf node
        for (std::vector<Particle*>::const_iterator i = root.bods.begin(); i != root.bods.end(); ++i)
        {
            if (&body != *i)
            {
                calc_acc(body, **i);
                numForceCalcs++;
            }
        }
    }
    else
    {
        // Not far enough away, and has child nodes, recurse on each child
        for (int i = 0; i < 8; ++i)
        {
            numForceCalcs += acc_barnes_hut(body, *(root.children[i]));
        }
    }

    return numForceCalcs;
}

// Calculate acceleration of body to center of mass
void calc_acc(Particle& body, const glm::vec4& com)
{
    glm::vec3 d = com.xyz() - body.pos; // distance
    float r2 = glm::dot(d,d); // distance squared
    float r3 = r2 * glm::sqrt(r2);

    float f_mag = (G*body.size*com.w) / (r3 + ETA);
    glm::vec3 f = d * f_mag; // force vector

    body.acc += f;
}

// Calculate acceleration of body to another body
void calc_acc(Particle& body, const Particle& other)
{
    glm::vec3 d = other.pos - body.pos; // distance
    float r2 = glm::dot(d,d); // distance squared
    float r3 = r2 * glm::sqrt(r2);

    float f_mag = (G*body.size*other.size) / (r3 + ETA);
    glm::vec3 f = d * f_mag; // force vector

    body.acc += f;
}

// Also resets accelerations
unsigned long ageKillResetParticles(double delta)
{
    unsigned long ParticlesCount = 0;
    for(int i=0; i<MaxParticles; i++){

        Particle& p = ParticlesContainer[i]; // shortcut

        if(p.life > 0.0f){

            // Decrease life
            p.life -= delta;

            if (p.life <= 0.0f){
                // Particles that just died will be put at the end of the buffer in SortParticles();
                p.cameradistance = -1.0f;
            }
            else
            {
                ParticlesCount++;
            }

        }

        // Reset acceleration
        p.acc.x = 0;
        p.acc.y = 0;
        p.acc.z = 0;
    }

    return ParticlesCount;
}

void createNewParticles(unsigned long ParticlesCount, double delta)
{
    // Generate 10 new particule each millisecond,
    // but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
    // newparticles will be huge and the next frame even longer.
    long particles_left = MaxParticles - ParticlesCount;
    
    if (particles_left > 0)
    {
        int newparticles = (int)(delta*NewParticleSpeed);
        // Don't push out too many if we lag
        if (newparticles > (int)(0.016f*NewParticleSpeed))
            newparticles = (int)(0.016f*NewParticleSpeed);
        // Don't go past max # of particles
        if (newparticles > particles_left)
            newparticles = particles_left;
    
        for(int i=0; i<newparticles; i++){
            int particleIndex = FindUnusedParticle();
            ParticlesContainer[particleIndex].life = 50.0f + 10.0f*((rand() % 1000)/1000.0f);
            ParticlesContainer[particleIndex].pos = glm::vec3(0,0,-20.0f);

            glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
            // Very bad way to generate a random direction; 
            // See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
            // combined with some user-controlled parameters (main direction, spread, etc)
            glm::vec3 randomdir = glm::vec3(
                (rand()%2000 - 1000.0f)/1000.0f,
                (rand()%2000 - 1000.0f)/1000.0f,
                (rand()%2000 - 1000.0f)/1000.0f
            );

            ParticlesContainer[particleIndex].pos += randomdir*5.0f; // Not directly in center
            
            // ParticlesContainer[particleIndex].vel = maindir + randomdir*spread;
            ParticlesContainer[particleIndex].vel = randomdir*SPREAD;


            // Very bad way to generate a random color
            ParticlesContainer[particleIndex].r = rand() % 256;
            ParticlesContainer[particleIndex].g = rand() % 256;
            ParticlesContainer[particleIndex].b = rand() % 256;
            ParticlesContainer[particleIndex].a = 255;// (rand() % 256) / 3;

            ParticlesContainer[particleIndex].size = ((rand()%1000)/2000.0f + 0.1f) / 5.f;
            
        }
    }
}

// Euler integration
unsigned long simulateEuler(double dt)
{
    unsigned int numForceCalcs= calc_all_acc_brute();
    updatePositionsVelocities(dt);
    return numForceCalcs;
}

// Leapfrog integration
unsigned long simulateLeapfrog(Octree* oct, double dt)
{
    updatePositions(0.5*dt);
    // unsigned int numForceCalcs= calc_all_acc_brute();
    unsigned int numForceCalcs= calc_all_acc_barnes_hut(*oct);
    updateVelocities(dt);
    updatePositions(0.5*dt);

    return numForceCalcs;
}