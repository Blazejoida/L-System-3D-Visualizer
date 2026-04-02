#include "Renderer.h"
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <algorithm>
#include <cmath>

static void computeBBox(const std::vector<Segment>& segs,
    float& cx, float& cy, float& cz, float& radius)
{
    if (segs.empty()) { cx = cy = cz = 0.f; radius = 1.f; return; }
    float mn[3] = { 1e9f, 1e9f, 1e9f };
    float mx[3] = { -1e9f,-1e9f,-1e9f };
    for (auto& s : segs) {
        const float pts[2][3] = { {s.a.x,s.a.y,s.a.z},{s.b.x,s.b.y,s.b.z} };
        for (auto& pt : pts)
            for (int i = 0; i < 3; i++) {
                mn[i] = std::min(mn[i], pt[i]);
                mx[i] = std::max(mx[i], pt[i]);
            }
    }
    cx = (mn[0] + mx[0]) * .5f; cy = (mn[1] + mx[1]) * .5f; cz = (mn[2] + mx[2]) * .5f;
    radius = std::max({ mx[0] - mn[0],mx[1] - mn[1],mx[2] - mn[2] }) * .5f + .001f;
}

void rendererInit()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void renderScene(const std::vector<Segment>& segs,
    const RenderState& rs)
{
    glViewport(0, 0, rs.winW, rs.winH);
    glClearColor(0.06f, 0.06f, 0.12f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float cx, cy, cz, radius;
    computeBBox(segs, cx, cy, cz, radius);

    float dist = radius * 2.8f / rs.camZoom;

    // Projection with near/far derived from scene size 
    double nearP = (double)dist * 0.001;
    if (nearP < 1e-6) nearP = 1e-6;
    double farP = (double)dist + (double)radius * 8.0;

    double asp = (rs.winH > 0) ? (double)rs.winW / rs.winH : 1.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, asp, nearP, farP);

    //  Camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, dist, 0, 0, 0, 0, 1, 0);
    glRotatef(rs.camRotX, 1, 0, 0);
    glRotatef(rs.camRotY, 0, 1, 0);
    glTranslatef(-cx, -cy, -cz);

    //  Floor grid
    {
        float floorY = cy - radius;
        float ext = radius * 2.2f;
        float step = radius * 0.22f;
        if (step < 0.001f) step = 0.001f;
        glLineWidth(1.0f);
        glColor3f(0.10f, 0.11f, 0.20f);
        glBegin(GL_LINES);
        for (float f = -ext; f <= ext + 1e-4f; f += step) {
            glVertex3f(cx + f, floorY, cz - ext); glVertex3f(cx + f, floorY, cz + ext);
            glVertex3f(cx - ext, floorY, cz + f); glVertex3f(cx + ext, floorY, cz + f);
        }
        glEnd();
    }

    //  Axis indicators (small, at root)
    {
        float al = radius * 0.08f;
        float ox = cx, oy = cy - radius, oz = cz;
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glColor3f(.85f, .2f, .2f); glVertex3f(ox, oy, oz); glVertex3f(ox + al, oy, oz);
        glColor3f(.2f, .85f, .2f); glVertex3f(ox, oy, oz); glVertex3f(ox, oy + al, oz);
        glColor3f(.2f, .4f, .9f);  glVertex3f(ox, oy, oz); glVertex3f(ox, oy, oz + al);
        glEnd();
    }

    //  Segments 
    for (auto& seg : segs) {
        glLineWidth(seg.width);
        glColor3f(seg.r, seg.g, seg.bv);
        glBegin(GL_LINES);
        glVertex3f(seg.a.x, seg.a.y, seg.a.z);
        glVertex3f(seg.b.x, seg.b.y, seg.b.z);
        glEnd();
    }
    glLineWidth(1.0f);
}