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
    glm::vec4 com; // center of mass (position, size)
    std::vector<Particle*> bods;

    Octree* children[8];

    Octree() : interior(false) {
        // Initially no children
        for (int i = 0; i < 8; ++i)
        {
            children[i] = NULL;
        }
    };
    Octree(const glm::vec3 center, const glm::vec3 half_width) : interior(false) {
        // Initially no children
        for (int i = 0; i < 8; ++i)
        {
            children[i] = NULL;
        }
        
        // Set up bbox with given values
        bbox.center = center;
        bbox.half_width = half_width;
    };

    ~Octree()
    {
        clear();
    }

    void clear()
    {
        // Delete all children recursively
        for (int i = 0; i < 8; ++i)
        {
            if (children[i])
            {
                children[i]->clear();
                delete children[i];
            }
        }
    }

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
    void insert(Particle* p, int maxDepth = 0, int depth = 0)
    {
        for (int i = 0; i < depth; ++i)
        {
            printf("-");
        }
        printf("INSERT <%g,%g,%g> : DEPTH %d, MAX %d, (interior? %d)\n", p->pos.x, p->pos.y, p->pos.z, depth, maxDepth, interior);
        if (depth > maxDepth+10)
        {
            return;
        }

        // 1 - interior, recurse on correct child
        if (interior)
        {
            printf("INTERIOR\n");
            int idx = getChildIndex(p->pos);
            if (depth >= maxDepth)
            {
                // Update COM with added object
                com = glm::vec4(  (com.xyz() * com.w + p->pos * p->size) / (com.w + p->size), com.w + p->size);
                bods.push_back(p);
            }
            else
            {
                children[idx]->insert(p, maxDepth, depth+1);
            }
        }
        else if (bods.empty())
        {
            printf("EMPTY\n");
            // 2 - leaf with no data, assign
            com = glm::vec4(p->pos, p->size);
            bods.push_back(p);
        }
        else
        {
            printf("SPLIT\n");
            // 3 - leaf with data, split up octant
            
            interior = true; // Becoming an interior node
            Particle* temp = bods.front();
            bods.clear();
            int idxA = getChildIndex(temp->pos);
            int idxB = getChildIndex(p->pos);

            // Create new children with current bounding boxes
            for (int i = 0; i < 8; ++i)
            {
                glm::vec3 child_center = bbox.center;
                child_center.x += bbox.half_width.x * (i&4 ? .5f : -.5f);
                child_center.y += bbox.half_width.y * (i&2 ? .5f : -.5f);
                child_center.z += bbox.half_width.z * (i&1 ? .5f : -.5f);
                children[i] = new Octree(child_center, bbox.half_width*.5f);
            }

            children[idxA]->insert(temp, maxDepth, depth+1);
            children[idxB]->insert(p, maxDepth, depth+1);
        }
    }

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

void generateOctree(Octree* root)
{
    // 1 - Delete all nodes
    root->clear();
    
    // 2 - Update bounds
    Bounds* b = calculateMainBounds(); //bbox of all particles
    root->setBounds(*b);

    // 3 - Add all particles
    for(int i=0; i<MaxParticles; i++)
    {
        Particle& p = ParticlesContainer[i]; // shortcut        
        if(p.life > 0.0f)
        {
            root->insert(&p, 10);
        }
    }
}