#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "LSystem.h"
#include "Turtle3D.h"
#include "Renderer.h"
#include <vector>

// Shared application state
struct AppState
{
    std::vector<LSystem>  presets;
    LSystem               current;
    std::vector<Segment>  segments;
    RenderState           rs;
    TurtleParams          tp = {};
    int                   presetIdx = 0;
    int                   iter = 4;
    float                 angleOver = 0.f;
    bool                  dirty = true;
    COLORREF              crTrunk = RGB(97, 56, 16);
    COLORREF              crLeaf = RGB(46, 179, 28);
};

class AppWindow
{
public:
    AppWindow();
    ~AppWindow();

    bool create(HINSTANCE hInst, AppState& state, int nShow);

    HWND hwndMain()  const { return m_hwndMain; }
    HWND hwndGL()    const { return m_hwndGL; }
    HWND hwndPanel() const { return m_hwndPanel; }

    void loadCustomGrammar(const LSystem& ls);
    void updateLabels();

private:
    static LRESULT CALLBACK mainProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK glProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK panelProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK swatchProc(HWND, UINT, WPARAM, LPARAM);
    // Subclassed edit proc — intercepts VK_RETURN directly on the edit control
    static LRESULT CALLBACK editIterProc(HWND, UINT, WPARAM, LPARAM);
    WNDPROC   m_editIterOldProc = NULL;  // original edit proc (for CallWindowProc)

    LRESULT onMainMsg(HWND, UINT, WPARAM, LPARAM);
    LRESULT onGLMsg(HWND, UINT, WPARAM, LPARAM);
    LRESULT onPanelMsg(HWND, UINT, WPARAM, LPARAM);

    void registerClasses(HINSTANCE hi);
    void createPanel();
    void createGLContext();
    void rebuild();
    void syncSwatches();
    bool pickColor(HWND parent, COLORREF& col);

    // Sync iter edit box - slider and state
    void applyIterEdit();

    HWND      m_hwndMain = NULL;
    HWND      m_hwndGL = NULL;
    HWND      m_hwndPanel = NULL;
    HDC       m_hDC = NULL;
    HGLRC     m_hRC = NULL;
    HINSTANCE m_hInst = NULL;

    // Panel controls
    HWND m_hCombo = NULL;
    HWND m_hSlIter = NULL;   // slider 1-20 (fast range)
    HWND m_hEditIter = NULL;   // editable spinbox for unlimited iter
    HWND m_hSlAngle = NULL;
    HWND m_hLblIter = NULL;
    HWND m_hLblAngle = NULL;
    HWND m_hLblSegs = NULL;
    HWND m_hLblDesc = NULL;
    HWND m_hChkCol = NULL;
    HWND m_hSwTrunk = NULL;
    HWND m_hSwLeaf = NULL;
    HWND m_hBtnTrunk = NULL;
    HWND m_hBtnLeaf = NULL;
    HWND m_hBtnRstCl = NULL;
    HWND m_hBtnCam = NULL;
    HWND m_hBtnGram = NULL;

    AppState* m_state = nullptr;
    bool      m_mouseDown = false;
    int       m_lastMX = 0;
    int       m_lastMY = 0;
    COLORREF  m_custColors[16] = {};

    static const int PANEL_W = 252;
    static const int SLIDER_MAX = 20;  // slider goes 1-20, edit box unlimited

    enum {
        ID_COMBO = 300,
        ID_SL_ITER = 301,
        ID_SL_ANG = 302,
        ID_BTN_TR = 303,
        ID_BTN_LF = 304,
        ID_BTN_CAM = 305,
        ID_BTN_RST = 306,
        ID_BTN_GRM = 307,
        ID_CHK_COL = 308,
        ID_SW_TR = 309,
        ID_SW_LF = 310,
        ID_GL = 311,
        ID_EDIT_ITER = 312,  // editable iteration field
    };
};