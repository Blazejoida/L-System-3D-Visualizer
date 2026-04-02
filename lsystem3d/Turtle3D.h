#pragma once
#include "Math3D.h"
#include "LSystem.h"
#include <vector>

struct Segment
{
    Vec3  a, b;
    float r, g, bv;
    float width;
};

struct TurtleState
{
    Vec3  pos, H, L, U;
    float stepLen;
    float width;
    int   depth;
};

struct TurtleParams
{
    float angleOverride; // 0 = use LSystem default
    float trunkR, trunkG, trunkB;
    float leafR, leafG, leafB;
    bool  useCustomColors;
};

std::vector<Segment> buildGeometry(const LSystem& ls,
    const std::string& str,
    const TurtleParams& tp);