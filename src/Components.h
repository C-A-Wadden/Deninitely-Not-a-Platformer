#pragma once

#include "Animation.h"
#include "Assets.h"

class Component
{
public:
    bool has = false;
};

class CTransform : public Component
{
public:
    Vec2 pos          = { 0.0, 0.0 };
    Vec2 prevPos      = { 0.0, 0.0 };
    Vec2 homePos      = { 0.0, 0.0 };
    Vec2 scale        = { 1.0, 1.0 };
    Vec2 velocity     = { 0.0, 0.0 };
    Vec2 acceleration = { 0.0, 0.0 };
    float angle       = 0;

    CTransform() {}
    CTransform(const Vec2 & p)
        : pos(p), homePos(p) {}
    CTransform(const Vec2 & p, const Vec2 & sp, const Vec2 & sc, float a)
        : pos(p), prevPos(p), velocity(sp), scale(sc), angle(a), homePos(p) {}

};

class CLifeSpan : public Component
{
public:
    int lifespan = 0;
    int frameCreated = 0;
    CLifeSpan() {}
    CLifeSpan(int duration, int frame) 
        : lifespan(duration), frameCreated(frame) {}
};

class CInput : public Component
{
public:
    bool up         = false;
    bool down       = false;
    bool left       = false;
    bool right      = false;
    bool shoot      = false;
    bool canShoot   = true;
    bool tryJump    = false;

    CInput() {}
};

class CBoundingBox : public Component
{
public:
    Vec2 size;
    Vec2 halfSize;
    bool blockMove = true;
    bool blockVision = true;
    CBoundingBox() {}
    CBoundingBox(const Vec2& s)
        : size(s), halfSize(s.x / 2, s.y / 2) {}
    CBoundingBox(const Vec2& s, bool m, bool v)
        : size(s), blockMove(m), blockVision(v), halfSize(s.x / 2, s.y / 2) {}
};

class CAnimation : public Component
{
public:
    Animation animation;
    bool repeat = false;
    CAnimation() {}
    CAnimation(const Animation & animation, bool r)
        : animation(animation), repeat(r) {}
};

class CGravity : public Component
{
public:
    float gravity = 0;
    CGravity() {}
    CGravity(float g) : gravity(g) {}
};

class CState : public Component
{
public:
    std::string state = "jumping";
    CState() {}
    CState(const std::string & s) : state(s) {}
};

class CAdvancedMovement : public Component
{
public:
    bool usesPhysics = false;
    int frameToAllowInfluence = 0;
    float jumpForgivenessTime = 0.1;
    float jumpForgivenessCounter = 0;
    float wallJumpForgivenessTime = 0.1;
    float wallJumpForgivenessCounter = 0;
    float jumpBufferTime = 0.1;
    float jumpBufferCounter = 0;
    int lastWallSide = 0;
    bool shotInAir = false;
    bool hasJumped = false;
    CAdvancedMovement() {}
    CAdvancedMovement(bool b) : usesPhysics(b) {}
};

class CPatrol : public Component
{
public:
    std::vector<Vec2> positions;
    size_t goalPosition = 1;
    size_t prevPosition = 0;
    float progress = 0;
    float speed = 0;
    bool playerOn = false;
    bool moving = true;
    bool trigger = false;
    CPatrol() {}
    CPatrol(std::vector<Vec2>& pos, float s, bool trig) : positions(pos), speed(s), trigger(trig)
    {
        moving = !trigger;
    }
};

class CFollowPlayer : public Component
{
public:
    Vec2 home = { 0, 0 };
    float speed = 0;
    CFollowPlayer() {}
    CFollowPlayer(Vec2 p, float s)
        : home(p), speed(s) {}
};

class CInvincibility : public Component
{
public:
    int iframes = 0;
    CInvincibility() {}
    CInvincibility(int f)
        : iframes(f) {}
};

class CHealth : public Component
{
public:
    float max = 1.0f;
    float current = 1.0f;
    CHealth() {}
    CHealth(float m, float c)
        : max(m), current(c) {}
};

class CDamage : public Component
{
public:
    float damage = 1;
    int atkRange = 120;
    CDamage() {}
    CDamage(float d)
        : damage(d) {}
    CDamage(float d, int a)
        : damage(d),
          atkRange(a) {}
};

class CPickUp : public Component
{
public:
    Vec2 startPos = Vec2(0, 0);
    std::vector<float> animTarget;
    bool goingUp = false;
    float animProgress = 0.5;
    int effect = 0;
    CPickUp() {}
    CPickUp(Vec2 pos, int eff)
        : startPos(pos),
          effect(eff)
    {
        animTarget.push_back(startPos.y + 5 * 1);
        animTarget.push_back(startPos.y + 5 * -1);
    }
};