#include "animator.hpp"
#include "Filesystem/file_manager.hpp"
#include "Debug/debug.hpp"
#include "Misc/string_helper.hpp"
#include "Time/time_helper.hpp"
#include <regex>

const std::map<std::string, animation_token> animator::tokens = {
        {"A", AT_A},
        {"AF", AT_AF},
        {"FL", AT_FL},
        {"F", AT_F}
};

void animator::load_sheet(texture* _sprite_texture, const std::string& _location) {
    ani_texture = _sprite_texture;

    std::vector<std::string> lines;
    if (!FileManager::read_file(_location, lines))
        return debug::alert("Animator - error failed to load file " + _location);

    animation* new_animation;

    for (int i = 0; i < lines.size(); i++) {
        if (lines[i].empty())
            continue;

        if (!regex_match(lines[i], std::regex("^[0-9a-zA-Z/ -]+$"))) {
            debug::alert(
                    "Animator - Invalid line " + std::to_string(i) + " with content  " + lines[i] + " in file " + _location);
            continue;
        }

        std::vector<std::string> data = explode(lines[i], ' ');

        auto token = animator::tokens.find(data[0]);

        if (token == animator::tokens.end()) {
            debug::alert("Animator - Invalid token " + data[0] + " in line " + std::to_string(i) + " in file " + _location);
            continue;
        }

        switch (token->second) {
            case AT_A:
                if (new_animation != nullptr)
                    animations.push_back(new_animation);
                new_animation = new animation();
                new_animation->type = ANIMATION_2D;
                new_animation->name = data[1];
                break;
            case AT_AF: {
                    int nFrames = stoi(data[4]);
                    int y = stoi(data[3]);
                    auto size = vector2(stoi(data[1]), stoi(data[2]));

                    for (int j = 0; j < nFrames; j++) {
                        animation_frame frame{};
                        frame.ani_sprite = new sprite(ani_texture, vector2(size.x * j, y * size.y), size);
                        new_animation->frames.push_back(frame);
                    }
                break;
                }
            case AT_FL:
                new_animation->frames[stoi(data[1])].length = stoi(data[2]);
                break;
            case AT_F:
                //TODO: this
                break;
        }
    }
    animations.push_back(new_animation);
}

void animator::play(const std::string &_name) {
    for (auto a : animations) {
        if (a->name == _name) {
            _playing_animation = a;
            _last_update = 0;
            _frame_index = 0;
        }
    }
}

void animator::load() {
    _renderer = get_component<sprite_renderer>();
}

void animator::update() {
    if (_renderer == nullptr || _playing_animation == nullptr)
        return;

    _last_update += time_helper::delta_time * 1000;

    if (_last_update < _playing_animation->frames[_frame_index].length)
        return;

    _last_update -= _playing_animation->frames[_frame_index].length;

    _frame_index++;

    if (_frame_index >= _playing_animation->frames.size())
        _frame_index = 0;

    _renderer->set_sprite(_playing_animation->frames[_frame_index].ani_sprite);
}