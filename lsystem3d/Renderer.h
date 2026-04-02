#pragma once
#include "Turtle3D.h"
#include <vector>
#include <string>

// Camera / view state (controlled via WinAPI in main)
struct RenderState
{
    float camRotX =  20.f;
    float camRotY =  30.f;
    float camZoom =   1.0f;
    int   winW    =  800;
    int   winH    =  600;
};

// Call once after context creation
void rendererInit();

// Main draw call - pure OpenGL, no GLUT
void renderScene(const std::vector<Segment>& segs,
                 const RenderState& rs);
