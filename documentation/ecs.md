# ECS (Entity Component System)

ECS implementation can be [found here.](../include/mars/engine/engine.hpp)

Our ECS implementation is implemented in such a way that you don't require **ANY** boilerplate, you simply add entities using a type, and it will automatically create and store components based on the member variables in the entity struct using static reflection.

## Usage

Here is an example of the ECS in action

```c++
// define components
struct transform {
    mars::vector3<float> pos;
    mars::vector3<float> scale;
};

struct entity_stats {
    float health;
};

struct player_stats {
    float score;
};

struct zombie_stats {
    bool has_arms;
};

// define entities
struct player_entity {
    transform transform;
    entity_stats e_stats;
    player_stats p_stats;
};

struct zombie_entity {
    transform transform;
    entity_stats e_stats;
    zombie_stats z_stats;
};

// define processes (first argument is always entity other arguments are the components can be a ref or a copy it doesn’t matter but if you care about performance please use a ref, also make it const if you dont want people changing it)
void process_entity(const mars::entity& entity, transform& _trans, entity_stats& _stats) {
    std::printf("%f, %f, %f\n", _trans.pos.x, _trans.pos.y, _trans.pos.z);
}

int main() {
    mars::engine engine;
    //request entities
    mars::entity p = engine.request_entity<player_entity>({ { { 1, 2, 3 } } });
    engine.request_entity<zombie_entity>({ { { 1, 2, 5 } } });

    // execute entities, this will output pos for each
    engine.process(&process_entity);
    //destroy player entity
    engine.entity_destroy(p);
    // execute entities again, p will obviously not be executed
    engine.process(&process_entity);
}

```