#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    // These collision algorithms came directly from the slides
    Vec2 delta = Vec2(abs(a->getComponent<CTransform>().pos.x - b->getComponent<CTransform>().pos.x),
        abs(a->getComponent<CTransform>().pos.y - b->getComponent<CTransform>().pos.y));
    float OX = (a->getComponent<CBoundingBox>().halfSize.x) + (b->getComponent<CBoundingBox>().halfSize.x) - delta.x;
    float OY = (a->getComponent<CBoundingBox>().halfSize.y) + (b->getComponent<CBoundingBox>().halfSize.y) - delta.y;
    if (OX <= 0 || OY <= 0)
    {
        return Vec2(0, 0);
    }
    return Vec2(OX, OY);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    // These collision algorithms came directly from the slides
    Vec2 delta = Vec2(abs(a->getComponent<CTransform>().prevPos.x - b->getComponent<CTransform>().prevPos.x),
        abs(a->getComponent<CTransform>().prevPos.y - b->getComponent<CTransform>().prevPos.y));
    float OX = (a->getComponent<CBoundingBox>().halfSize.x) + (b->getComponent<CBoundingBox>().halfSize.x) - delta.x;
    float OY = (a->getComponent<CBoundingBox>().halfSize.y) + (b->getComponent<CBoundingBox>().halfSize.y) - delta.y;
    return Vec2(OX, OY);
}

Intersect Physics::LineIntersect(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d)
{
    // returns intersect object {bool interests, Vec2 intersentPoint }

    Vec2 r = (b - a);
    Vec2 s = (d - c);
    float rxs = r * s;
    Vec2 cma = c - a;
    float t = (cma * s) / rxs;
    float u = (cma * r) / rxs;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
        return { true, Vec2(a.x + t * r.x, a.y + t * r.y) };
    else
        return { false, Vec2(0,0) };
}

bool Physics::EntityIntersect(const Vec2& a, const Vec2& b, std::shared_ptr<Entity> e)
{
    // returns bool line intsects entity

    auto& bb = e->getComponent<CBoundingBox>();
    auto pos = e->getComponent<CTransform>().pos;
    if (!bb.blockVision) return false;

    Vec2 bbA = (pos - bb.halfSize);
    Vec2 bbB = Vec2(pos.x - bb.halfSize.x, pos.y + bb.halfSize.y);
    Vec2 bbC = Vec2(pos.x + bb.halfSize.x, pos.y - bb.halfSize.y);
    Vec2 bbD = (pos + bb.halfSize);

    if (LineIntersect(a, b, bbA, bbB).result) return true;
    if (LineIntersect(a, b, bbA, bbC).result) return true;
    if (LineIntersect(a, b, bbC, bbC).result) return true;
    if (LineIntersect(a, b, bbB, bbD).result) return true;

    return false;
}

bool Physics::IsInside(const Vec2& pos, std::shared_ptr<Entity> e)
{
    Vec2 ePos = e->getComponent<CTransform>().pos;
    Vec2 bBoxSize = e->getComponent<CBoundingBox>().halfSize;
    // Return whether or not the point specified is within the bounding box of the entity specified
    if ((pos.x > ePos.x - bBoxSize.x) && (pos.x < ePos.x + bBoxSize.x) && (pos.y > ePos.y - bBoxSize.y) && (pos.y < ePos.y + bBoxSize.y))
    {
        return true;
    }
    return false;
}