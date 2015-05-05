// Contains functions for simulating particles

// Physics constants
#define G 2.0f
#define ETA 0.01f

const int MaxParticles = 1000;
const float NewParticleSpeed = 10000000.0f;
int LastUsedParticle = 0;


// CPU representation of a particle
struct Particle{
    glm::vec3 pos, vel, acc;
    unsigned char r,g,b,a; // Color
    float size, angle, weight;
    float life; // Remaining life of the particle. if <0 : dead and unused.
    float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

    bool operator<(const Particle& that) const {
        // Sort in reverse order : far particles drawn first.
        return this->cameradistance > that.cameradistance;
    }
};


Particle ParticlesContainer[MaxParticles];

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
unsigned long calculateAccelerations()
{
    // Simulate all particles
    unsigned long numForceCalcs = 0;
    for(int i=0; i<MaxParticles; i++){

        Particle& p = ParticlesContainer[i]; // shortcut

        // Ignore dead particles (they'll be reset later)
        if(p.life > 0.0f)
        {
            // Simulate simple physics : gravity only, no collisions
            // p.vel += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f;

            // glm::vec3 acc(0.0f,0.0f,0.0f);

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
            ParticlesContainer[particleIndex].life = 20.0f + 10.0f*((rand() % 1000)/1000.0f); // This particle will live 5 seconds.
            ParticlesContainer[particleIndex].pos = glm::vec3(0,0,-20.0f);

            float spread = 1.5f;
            glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
            // Very bad way to generate a random direction; 
            // See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
            // combined with some user-controlled parameters (main direction, spread, etc)
            glm::vec3 randomdir = glm::vec3(
                (rand()%2000 - 1000.0f)/1000.0f,
                (rand()%2000 - 1000.0f)/1000.0f,
                (rand()%2000 - 1000.0f)/1000.0f
            );

            ParticlesContainer[particleIndex].pos += randomdir; // Not directly in center
            
            // ParticlesContainer[particleIndex].vel = maindir + randomdir*spread;
            ParticlesContainer[particleIndex].vel = randomdir*spread;


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
    unsigned long numForceCalcs= calculateAccelerations();
    updatePositionsVelocities(dt);
    return numForceCalcs;
}

// Leapfrog integration
unsigned long simulateLeapfrog(double dt)
{
    updatePositions(0.5*dt);
    unsigned long numForceCalcs= calculateAccelerations();
    updateVelocities(dt);
    updatePositions(0.5*dt);

    return numForceCalcs;
}