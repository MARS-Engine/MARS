#ifndef __ANIMATOR__
#define __ANIMATOR__

#include "Type/sprite.hpp"
#include "Components/Graphics/2D/sprite_renderer.hpp"
#include "Multithread/component.hpp"

#include <string>
#include <vector>
#include <map>



enum animation_type {
    ANIMATION_2D,
    ANIMATION_3D
};

enum animation_token {
    AT_A,
    AT_AF,
    AT_FL,
    AT_F,
};

struct animation_frame {
public:
    sprite* ani_sprite = nullptr;
    //length (ms)
    int length;
};

struct animation {
public:
    animation_type type;
    std::string name;
    std::vector<animation_frame> frames;
};

class animator : public  component<animator> {
private:
    float _last_update = 0.0f;
    size_t _frame_index = 0;
    animation* _playing_animation;

    sprite_renderer* _renderer = nullptr;
public:
    static const std::map<std::string, animation_token> tokens;
    texture* ani_texture;
    std::vector<animation*> animations;

    void load_sheet(texture* _sprite_texture, const std::string& _location);
    void play(const std::string& _name);
    void load() override;
    void update() override;
};

#endif