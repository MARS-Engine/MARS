#ifndef __ANIMATOR__
#define __ANIMATOR__

#include "Type/Sprite.hpp"
#include "Components/Graphics/2D/SpriteRenderer.hpp"
#include "Multithread/Component.hpp"

#include <string>
#include <vector>
#include <map>

using namespace std;

enum AnimationType {
    ANIMATION_2D,
    ANIMATION_3D
};

enum AnimationToken {
    AT_A,
    AT_AF,
    AT_FL,
    AT_F,
};

struct AnimationFrame {
public:
    Sprite* sprite = nullptr;
    //length (ms)
    int length;
};

struct Animation {
public:
    AnimationType type;
    string name;
    vector<AnimationFrame> frames;
};

class Animator : public  Component<Animator> {
private:
    float _lastUpdate = 0.0f;
    size_t frameIndex = 0;
    Animation* playingAnimation;

    SpriteRenderer* renderer = nullptr;
public:
    static const map<string, AnimationToken> tokens;
    Texture* texture;
    vector<Animation*> animations;

    void LoadSheet(Texture* texture, const string& location);
    void Play(const string& name);
    void Load() override;
    void Update() override;
};

#endif