#include "Animation.h"
#include <cmath>

Animation::Animation()
{
}

Animation::Animation(const std::string & name, const sf::Texture & t)
    : Animation(name, t, 1, 0, 0)
{

}

Animation::Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed, bool override)
    : m_name        (name)
    , m_sprite      (t)
    , m_frameCount  (frameCount)
    , m_currentFrame(0)
    , m_speed       (speed)
    , m_override    (override)
{
    m_size = Vec2((float)t.getSize().x / frameCount, (float)t.getSize().y);
         if (m_name.substr(0, 5) == "Guard") m_sprite.setOrigin((m_size.x / 5.0f)-5, (m_size.y / 2.0f)-10);
    else if (m_name.substr(0, 4) == "Boss")  m_sprite.setOrigin(32, 25);
    else                                     m_sprite.setOrigin(m_size.x / 2.0f, m_size.y / 2.0f);
    m_sprite.setTextureRect(sf::IntRect(std::floor(m_currentFrame) * m_size.x, 0, m_size.x, m_size.y));
}

// updates the animation to show the next frame, depending on its speed
// animation loops when it reaches the end
void Animation::update()
{
    // Update the animation frame over time
    m_currentFrame++;
    float frame = (m_speed != 0) ? (m_currentFrame / m_speed) % m_frameCount : 0;
    m_sprite.setTextureRect(sf::IntRect(std::floor(frame) * m_size.x, 0, m_size.x, m_size.y));
}

const Vec2 & Animation::getSize() const
{
    return m_size;
}

const std::string & Animation::getName() const
{
    return m_name;
}

sf::Sprite & Animation::getSprite()
{
    return m_sprite;
}

bool Animation::getOverride() 
{
    return m_override;
}

bool Animation::hasEnded() const
{
    // If an animation is on the last frame and doesn't loop, it has ended
    float frame = (m_speed != 0) ? (m_currentFrame / m_speed) % m_frameCount : 0;
    if (frame == m_frameCount - 1)
    {
        return true;
    }
    return false;
}

float Animation::getFrame()
{
    return (m_speed != 0) ? (m_currentFrame / m_speed) % m_frameCount : 0;
}