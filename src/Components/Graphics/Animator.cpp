#include "Animator.hpp"
#include "Filesystem/FileManager.hpp"
#include "Debug/Debug.hpp"
#include "Misc/StringHelper.hpp"
#include "Time/TimeHelper.hpp"
#include <regex>

const map<string, AnimationToken> Animator::tokens = {
        {"A", AT_A},
        {"AF", AT_AF},
        {"FL", AT_FL},
        {"F", AT_F}
};

void Animator::LoadSheet(Texture* _texture, const string& location) {
    texture = _texture;

    vector<string> lines;
    if (!FileManager::ReadFile(location, lines))
        return Debug::Alert("Animator - Error failed to load file " + location);

    Animation* animation;

    for (int i = 0; i < lines.size(); i++) {
        if (lines[i].empty())
            continue;

        if (!regex_match(lines[i], regex("^[0-9a-zA-Z/ -]+$"))) {
            Debug::Alert("Animator - Invalid line " + to_string(i) + " with content  " + lines[i] + " in file " + location);
            continue;
        }

        vector<string> data = Explode(lines[i], ' ');

        auto token = Animator::tokens.find(data[0]);

        if (token == Animator::tokens.end()) {
            Debug::Alert("Animator - Invalid token " + data[0] + " in line " + to_string(i) + " in file " + location);
            continue;
        }

        switch (token->second) {
            case AT_A:
                if (animation != nullptr)
                    animations.push_back(animation);
                animation = new Animation();
                animation->type = ANIMATION_2D;
                animation->name = data[1];
                break;
            case AT_AF: {
                    int nFrames = stoi(data[4]);
                    int y = stoi(data[3]);
                    auto size = Vector2(stoi(data[1]), stoi(data[2]));

                    for (int j = 0; j < nFrames; j++) {
                        AnimationFrame frame{};
                        frame.sprite = new Sprite(texture, Vector2(size.x * j, y * size.y), size);
                        animation->frames.push_back(frame);
                    }
                break;
                }
            case AT_FL:
                animation->frames[stoi(data[1])].length = stoi(data[2]);
                break;
            case AT_F:
                //TODO: this
                break;
        }
    }
    animations.push_back(animation);
}

void Animator::Play(const std::string &name) {
    for (auto a : animations) {
        if (a->name == name) {
            playingAnimation = a;
            _lastUpdate = 0;
            frameIndex = 0;
        }
    }
}

void Animator::Load() {
    renderer = GetComponent<SpriteRenderer>();
}

void Animator::Update() {
    if (renderer == nullptr || playingAnimation == nullptr)
        return;

    _lastUpdate += TimeHelper::deltaTime * 1000;

    if (_lastUpdate < playingAnimation->frames[frameIndex].length)
        return;

    _lastUpdate -= playingAnimation->frames[frameIndex].length;

    frameIndex++;

    if (frameIndex >= playingAnimation->frames.size())
        frameIndex = 0;

    renderer->SetSprite(playingAnimation->frames[frameIndex].sprite);
}