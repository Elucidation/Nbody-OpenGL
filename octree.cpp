// Used for oct-tree decomposition & Barnes-Hut algorithm in nbody sim

// AABB bounding box
struct Bounds
{
  glm::vec3 center, half_width;
};

class Octree
{
    Bounds* bbox;
public:
    bool interior;
    glm::vec4 com; // center of mass (position, size)
    std::vector<Particle*> bods;

    Octree* children[8];

    Octree()
    {
        Init();
    };
    Octree(const glm::vec3& center, const glm::vec3& half_width)
    {
        Init();
        
        // Set up bbox with given values
        bbox->center = glm::vec3(center);
        bbox->half_width = glm::vec3(half_width);
    };

    ~Octree()
    {
        clear();
    }

    void Init()
    {
        bbox = new Bounds();

        // Initially a leaf
        interior = false;

        // Initially no children
        for (int i = 0; i < 8; ++i)
        {
            children[i] = NULL;
        }
    }

    void clear()
    {
        delete bbox;

        bods.clear();
        
        // Delete all children recursively
        for (int i = 0; i < 8; ++i)
        {
            if (children[i])
            {
                delete children[i];
            }
        }
    }

    Bounds* getBounds()
    {
        return bbox;
    }

    void setBounds(Bounds* b)
    {
        bbox = b;
    }

    bool isInBounds(const glm::vec3& point)
    {
        return glm::all(glm::lessThan(glm::abs(point - bbox->center), bbox->half_width));
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
    void insert(Particle* p, int maxDepth = 0, int depth = 0)
    {
        if (depth > maxDepth+10)
        {
            return;
        }

        // 1 - interior, recurse on correct child
        if (interior)
        {
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
            // 2 - leaf with no data, assign
            com = glm::vec4(p->pos, p->size);
            bods.push_back(p);
        }
        else
        {
            // 3 - leaf with data, split up octant
            
            interior = true; // Becoming an interior node
            Particle* temp = bods.front();
            bods.clear();
            int idxA = getChildIndex(temp->pos);
            int idxB = getChildIndex(p->pos);

            // Create new children with current bounding boxes
            for (int i = 0; i < 8; ++i)
            {
                glm::vec3 child_center = glm::vec3(bbox->center);
                child_center.x += bbox->half_width.x * (i&4 ? .5f : -.5f);
                child_center.y += bbox->half_width.y * (i&2 ? .5f : -.5f);
                child_center.z += bbox->half_width.z * (i&1 ? .5f : -.5f);
                glm::vec3 child_half_width = bbox->half_width*.5f;
                children[i] = new Octree(child_center, child_half_width );
            }

            children[idxA]->insert(temp, maxDepth, depth+1);
            children[idxB]->insert(p, maxDepth, depth+1);
        }
    }
    
    // otree.getChildIndex(glm::vec3 point)
    //      returns idx 0-7 for correct octant
    int getChildIndex(const glm::vec3& point)
    {
        int idx = 0;
        // octants from -x,-y,-z to x,y,z
        if (point.x >= bbox->center.x)
            idx |= 1 << 2;
        if (point.y >= bbox->center.y)
            idx |= 1 << 1;
        if (point.z >= bbox->center.z)
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

Octree* generateOctree()
{
    // printf("GENERATE OCTREE\n");
    // 1 - Delete all nodes
    Octree* root = new Octree();
    
    // 2 - Update bounds
    Bounds* b = calculateMainBounds(); //bbox of all particles
    root->setBounds(b);

    // printf("Main Bounds: <%g %g %g>, <%g %g %g>\n", b->center.x, b->center.y, b->center.z,
    //                                   b->half_width.x,b->half_width.y,b->half_width.z);

    // 3 - Add all particles
    for(int i=0; i<MaxParticles; i++)
    {
        Particle* p = &(ParticlesContainer[i]); // shortcut        
        if(p->life > 0.0f)
        {
            root->insert(p, 10);
        }
    }
    // printf("DONE GENERATING OCTREE\n---\n");
    return root;
}