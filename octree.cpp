// Used for oct-tree decomposition & Barnes-Hut algorithm in nbody sim

// AABB bounding box
struct Bounds
{
  glm::vec3 center, half_width;
};

class Octree
{
    Bounds bbox;
public:
    bool interior;
    Particle* p;
    Octree* children[8];

    Octree() {
        // Bounds bbox;

        // Start off as a leaf node with no data
        interior = false;

        // Initially no children
        for (int i = 0; i < 8; ++i)
        {
            children[i] = NULL;
        }
    };
    Octree(glm::vec3 center, glm::vec3 half_width) {
        Octree();
        
        // Set up bbox with given values
        bbox.center = center;
        bbox.half_width = half_width;
    };

    ~Octree();

    Bounds& getBounds()
    {
        return bbox;
    }

    void setBounds(Bounds& b)
    {
        bbox = b;
    }

    bool isInBounds(const glm::vec3 point)
    {
        return glm::all(glm::lessThan(glm::abs(point - bbox.center), bbox.half_width));
    }

    // octree is interior node (has children) or leaf node (no children/unsplit)
    //
    // otree.insert(node)
    // 1 - otree is interior
    //      find correct child and recursively call insert(child)
    // 2 - otree is leaf with nothing assigned
    //      assign data to leaf and done
    // 3 - otree is leaf with data
    //      store data in temp
    //      create 8 children, find correct child for both new data & temp data
    //      recurse call insert(childA,data) & insert(childB, temp_data)
    //
    // otree.findChildIndex(glm::vec3 point)
    //      returns idx 0-7 for correct quadrant

    int getChildIndex(const glm::vec3& point)
    {
        int idx = 0;
        // quadrants from -x,-y,-z to x,y,z
        if (point.x >= bbox.center.x)
            idx |= 1 << 2;
        if (point.y >= bbox.center.y)
            idx |= 1 << 1;
        if (point.z >= bbox.center.z)
            idx |= 1 << 0;
        return idx;
    }
};


Bounds* calculateMainBounds()
{
    glm::vec3 bmin = glm::vec3(10000,10000,10000);
    glm::vec3 bmax = glm::vec3(-100000,-100000,-100000);
    for(int i=0; i<MaxParticles; i++)
    {
        Particle& p = ParticlesContainer[i]; // shortcut        
        if(p.life > 0.0f)
        {
            bmin = min(bmin, p.pos);
            bmax = max(bmax, p.pos);
        }
    }

    Bounds* bbox = new Bounds();
    bbox->center = (bmin + bmax)/2.0f;
    bbox->half_width = (bmax-bmin)/2.0f;
    
    return bbox;
}