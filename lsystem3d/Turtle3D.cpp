#include "Turtle3D.h"
#include <stack>
#include <algorithm>

//  ABoP turtle symbol set (p.19, Appendix C):
//
//  DRAW symbols (move forward AND draw segment):
//    F G    (all other letters are NON-DRAWING node symbols)
//
//  MOVE symbols (move forward, no segment):
//    f
//
//  ROTATION symbols:
//    +  yaw  left   (RU +delta)
//    -  yaw  right  (RU -delta)
//    &  pitch down  (RL +delta)
//    ^  pitch up    (RL -delta)
//    \  roll left   (RH +delta) 
//    /  roll right  (RH -delta) 
//    |  U-turn (RU 180)
//
//  STACK:
//    [  push state
//    ]  pop state
//
//  All other letters (A-E, H-Z except F G f) are IGNORED
//  by the turtle — they exist only as grammar rewriting nodes.

static void interpColor(const LSystem& ls, const TurtleParams& tp,
    int depth, int maxDepth,
    float& r, float& g, float& b)
{
    float t = (maxDepth > 0) ? float(depth) / float(maxDepth) : 0.f;
    t = std::min(std::max(t, 0.f), 1.f);

    BranchColor c0 = ls.colTrunk;
    BranchColor c1 = ls.colBranch;
    BranchColor c2 = ls.colLeaf;

    if (tp.useCustomColors) {
        c0 = { tp.trunkR, tp.trunkG, tp.trunkB };
        c2 = { tp.leafR,  tp.leafG,  tp.leafB };
        c1 = { (c0.r + c2.r) * .5f,(c0.g + c2.g) * .5f,(c0.b + c2.b) * .5f };
    }

    if (t < 0.5f) {
        float s = t / 0.5f;
        r = c0.r + (c1.r - c0.r) * s;
        g = c0.g + (c1.g - c0.g) * s;
        b = c0.b + (c1.b - c0.b) * s;
    }
    else {
        float s = (t - 0.5f) / 0.5f;
        r = c1.r + (c2.r - c1.r) * s;
        g = c1.g + (c2.g - c1.g) * s;
        b = c1.b + (c2.b - c1.b) * s;
    }
}

std::vector<Segment> buildGeometry(const LSystem& ls,
    const std::string& str,
    const TurtleParams& tp)
{
    std::vector<Segment> segs;
    segs.reserve(str.size() / 4);

    float ang = (tp.angleOverride > 0.f) ? tp.angleOverride : ls.angle;

    TurtleState cur;
    cur.pos = Vec3(0, 0, 0);
    cur.H = Vec3(0, 1, 0);   // heading: up Y
    cur.L = Vec3(-1, 0, 0);  // left:   -X
    cur.U = Vec3(0, 0, 1);   // up:     +Z
    cur.stepLen = ls.stepLen;
    cur.width = 2.8f;
    cur.depth = 0;

    std::stack<TurtleState> stk;

    // count max nesting depth for colour interpolation
    int maxDepth = 0;
    for (char c : str) if (c == '[') ++maxDepth;
    maxDepth = std::min(maxDepth, 18);

    for (char c : str)
    {
        switch (c)
        {
            // DRAW: only F and G 
        case 'F': case 'G': {
            Vec3 next = cur.pos + cur.H * cur.stepLen;
            Segment seg;
            seg.a = cur.pos;
            seg.b = next;
            seg.width = cur.width;
            interpColor(ls, tp, cur.depth, maxDepth,
                seg.r, seg.g, seg.bv);
            segs.push_back(seg);
            cur.pos = next;
            break;
        }

                // MOVE without draw 
        case 'f':
            cur.pos = cur.pos + cur.H * cur.stepLen;
            break;

            // YAW (rotate around U) 
        case '+':
            cur.H = rotateAround(cur.H, cur.U, ang);
            cur.L = rotateAround(cur.L, cur.U, ang);
            break;
        case '-':
            cur.H = rotateAround(cur.H, cur.U, -ang);
            cur.L = rotateAround(cur.L, cur.U, -ang);
            break;

            //  PITCH (rotate around L) 
        case '&':
            cur.H = rotateAround(cur.H, cur.L, ang);
            cur.U = rotateAround(cur.U, cur.L, ang);
            break;
        case '^':
            cur.H = rotateAround(cur.H, cur.L, -ang);
            cur.U = rotateAround(cur.U, cur.L, -ang);
            break;

            //  ROLL (rotate around H)
        case '\\':
            cur.L = rotateAround(cur.L, cur.H, ang);
            cur.U = rotateAround(cur.U, cur.H, ang);
            break;
        case '/':
            cur.L = rotateAround(cur.L, cur.H, -ang);
            cur.U = rotateAround(cur.U, cur.H, -ang);
            break;

            //  U-TURN 
        case '|':
            cur.H = rotateAround(cur.H, cur.U, 180.f);
            cur.L = rotateAround(cur.L, cur.U, 180.f);
            break;

            //  STACK
        case '[':
            cur.depth++;
            cur.width = std::max(0.25f, cur.width * 0.68f);
            cur.stepLen = cur.stepLen * ls.stepScale;
            stk.push(cur);
            break;
        case ']':
            if (!stk.empty()) { cur = stk.top(); stk.pop(); }
            break;

            // All other letters: ignored (grammar nodes, not turtle actions)
        default:
            break;
        }
    }
    return segs;
}