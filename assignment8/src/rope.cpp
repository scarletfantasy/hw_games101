#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"
const float damping=0.8f;
const float damping_fractor=0.00005;
namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        auto dis=(end-start)/(num_nodes-1);
        for(int i=0;i<num_nodes;++i)
        {
            auto curmass=new Mass(start+i*dis,node_mass,false);
            masses.push_back(curmass);
        }
        for(int i=0;i<num_nodes-1;++i)
        {
            auto curspring=new Spring(masses[i],masses[i+1],k);
            springs.push_back(curspring);
        }
//        Comment-in this part when you implement the constructor
        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            auto a=s->m1;
            auto b=s->m2;
            auto dir=b->position-a->position;
            auto fab=s->k*dir/dir.norm()*(dir.norm()-s->rest_length);
            a->forces+=fab;
            b->forces+=-1.0f*fab;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position

                // TODO (Part 2): Add global damping
                auto acc=(m->forces+gravity)/m->mass;
                auto vel=m->velocity*damping+acc*delta_t;
                m->velocity=vel;
                auto x=m->position+m->velocity*delta_t;
                
                m->position=x;

            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            auto a=s->m1;
            auto b=s->m2;
            auto dir=b->position-a->position;
            auto fab=s->k*dir/dir.norm()*(dir.norm()-s->rest_length);
            a->forces+=fab;
            b->forces+=-1.0f*fab;
        
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                auto acc=(m->forces+gravity)/m->mass;
                m->position=temp_position+(1.0-damping_fractor)*(temp_position-m->last_position)+acc*delta_t*delta_t;
                m->last_position=temp_position;
                // TODO (Part 3.1): Set the new position of the rope mass
                
                // TODO (Part 4): Add global Verlet damping
            }
            m->forces = Vector2D(0, 0);
        }
    }
}
