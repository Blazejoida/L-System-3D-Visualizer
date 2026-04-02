#pragma once
#include <string>
#include <map>
#include <vector>

struct BranchColor { float r, g, b; };

struct LSystem
{
    std::string                name;
    std::string                axiom;
    std::map<char, std::string> rules;
    float       angle;
    float       stepLen;
    float       stepScale;
    BranchColor colTrunk;
    BranchColor colBranch;
    BranchColor colLeaf;
    std::string description;
};

std::vector<LSystem> buildPresets();
std::string          expandLSystem(const LSystem& ls, int iters);