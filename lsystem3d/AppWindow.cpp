#include "AppWindow.h"
#include "GrammarDialog.h"
#include <commctrl.h>
#include <commdlg.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>

#define INST_FROM(hwnd) ((AppWindow*)GetWindowLongPtrA(hwnd,GWLP_USERDATA))

LRESULT CALLBACK AppWindow::mainProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CREATE) {
        SetWindowLongPtrA(hwnd, GWLP_USERDATA,
            (LONG_PTR)((CREATESTRUCTA*)lp)->lpCreateParams);
        return 0;
    }
    auto* s = INST_FROM(hwnd); return s ? s->onMainMsg(hwnd, msg, wp, lp)
        : DefWindowProcA(hwnd, msg, wp, lp);
}
LRESULT CALLBACK AppWindow::glProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CREATE) {
        SetWindowLongPtrA(hwnd, GWLP_USERDATA,
            (LONG_PTR)((CREATESTRUCTA*)lp)->lpCreateParams);
        return 0;
    }
    auto* s = INST_FROM(hwnd); return s ? s->onGLMsg(hwnd, msg, wp, lp)
        : DefWindowProcA(hwnd, msg, wp, lp);
}
LRESULT CALLBACK AppWindow::panelProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CREATE) {
        SetWindowLongPtrA(hwnd, GWLP_USERDATA,
            (LONG_PTR)((CREATESTRUCTA*)lp)->lpCreateParams);
        return 0;
    }
    auto* s = INST_FROM(hwnd); return s ? s->onPanelMsg(hwnd, msg, wp, lp)
        : DefWindowProcA(hwnd, msg, wp, lp);
}
LRESULT CALLBACK AppWindow::swatchProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_PAINT) {
        HWND panel = GetParent(hwnd);
        auto* self = (AppWindow*)GetWindowLongPtrA(panel, GWLP_USERDATA);
        if (!self) return DefWindowProcA(hwnd, msg, wp, lp);
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        COLORREF col = (hwnd == self->m_hSwTrunk) ? self->m_state->crTrunk
            : self->m_state->crLeaf;
        HBRUSH br = CreateSolidBrush(col);
        FillRect(hdc, &rc, br); DeleteObject(br);
        FrameRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
        EndPaint(hwnd, &ps); return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT CALLBACK AppWindow::editIterProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto* self = (AppWindow*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    if (!self) return DefWindowProcA(hwnd, msg, wp, lp);

    switch (msg)
    {
    case WM_KEYDOWN:
        if (wp == VK_RETURN) {
            self->applyIterEdit();
            // Move focus away so the edit doesn't keep blinking cursor
            SetFocus(self->m_hwndPanel);
            return 0;
        }
        if (wp == VK_ESCAPE) {
            // Revert to current state value and lose focus
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", self->m_state->iter);
            SetWindowTextA(hwnd, buf);
            SetFocus(self->m_hwndPanel);
            return 0;
        }
        break;

    case WM_CHAR:
        // Suppress the system beep that Windows plays when
        // an edit inside a dialog-like window receives VK_RETURN.
        if (wp == '\r' || wp == '\n' || wp == 27)
            return 0;
        break;
    }

    // Forward everything else to the original edit proc
    return CallWindowProcA(self->m_editIterOldProc, hwnd, msg, wp, lp);
}

AppWindow::AppWindow() {}
AppWindow::~AppWindow() {}

void AppWindow::registerClasses(HINSTANCE hi)
{
    WNDCLASSA wc = {};
    auto reg = [&](const char* nm, WNDPROC proc, HBRUSH bg) {
        wc = {}; wc.lpfnWndProc = proc; wc.hInstance = hi;
        wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
        wc.hbrBackground = bg; wc.lpszClassName = nm;
        RegisterClassA(&wc);
        };
    // Swatch — white bg, painted manually
    reg("SwCls6", swatchProc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    // Panel — dark bg
    reg("PanCls6", panelProc, CreateSolidBrush(RGB(24, 24, 32)));
    // GL child — no background
    wc = {}; wc.lpfnWndProc = glProc; wc.hInstance = hi;
    wc.hCursor = LoadCursorA(NULL, IDC_CROSS);
    wc.hbrBackground = NULL; wc.lpszClassName = "GLCls6";
    wc.style = CS_OWNDC; RegisterClassA(&wc);
    // Main
    wc = {}; wc.lpfnWndProc = mainProc; wc.hInstance = hi;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "MainCls6";
    wc.hIcon = LoadIconA(NULL, IDI_APPLICATION); RegisterClassA(&wc);
}

bool AppWindow::create(HINSTANCE hInst, AppState& state, int nShow)
{
    m_hInst = hInst; m_state = &state;
    registerClasses(hInst);

    int SW = GetSystemMetrics(SM_CXSCREEN), SH = GetSystemMetrics(SM_CYSCREEN);
    int WW = 1280, WH = 820;
    m_hwndMain = CreateWindowExA(0, "MainCls6",
        "L-System 3D Visualizer  |  Windows API + OpenGL",
        WS_OVERLAPPEDWINDOW,
        (SW - WW) / 2, (SH - WH) / 2, WW, WH,
        NULL, NULL, hInst, (LPVOID)this);
    if (!m_hwndMain) return false;

    RECT rc; GetClientRect(m_hwndMain, &rc);
    int glW = rc.right - PANEL_W, glH = rc.bottom;
    m_hwndGL = CreateWindowExA(0, "GLCls6", "",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        PANEL_W, 0, glW, glH,
        m_hwndMain, (HMENU)(INT_PTR)ID_GL, hInst, (LPVOID)this);
    if (!m_hwndGL) return false;
    m_state->rs.winW = glW; m_state->rs.winH = glH;

    createGLContext();
    createPanel();
    syncSwatches();
    updateLabels();
    ShowWindow(m_hwndMain, nShow);
    UpdateWindow(m_hwndMain);
    return true;
}

void AppWindow::createGLContext()
{
    m_hDC = GetDC(m_hwndGL);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd); pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA; pfd.cColorBits = 32; pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int fmt = ChoosePixelFormat(m_hDC, &pfd);
    if (fmt) SetPixelFormat(m_hDC, fmt, &pfd);
    m_hRC = wglCreateContext(m_hDC);
    wglMakeCurrent(m_hDC, m_hRC);
    rendererInit();
}

void AppWindow::createPanel()
{
    RECT rc; GetClientRect(m_hwndMain, &rc);
    m_hwndPanel = CreateWindowExA(0, "PanCls6", "",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, PANEL_W, rc.bottom,
        m_hwndMain, NULL, m_hInst, (LPVOID)this);

    HFONT hF = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HINSTANCE hi = m_hInst;
    HWND p = m_hwndPanel;
    int W = PANEL_W - 20;
    HWND ctl;

    int y = 8;

    // --- PRESET ---
    ctl = CreateWindowA("STATIC", "=== PRESET ===",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 18, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE); y += 20;

    m_hCombo = CreateWindowA("COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        10, y, W, 220, p, (HMENU)(INT_PTR)ID_COMBO, hi, NULL);
    SendMessage(m_hCombo, WM_SETFONT, (WPARAM)hF, TRUE);
    for (auto& ls : m_state->presets)
        SendMessageA(m_hCombo, CB_ADDSTRING, 0, (LPARAM)ls.name.c_str());
    SendMessage(m_hCombo, CB_SETCURSEL, 0, 0);
    y += 30;

    m_hLblDesc = CreateWindowA("STATIC", "",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 34, p, NULL, hi, NULL);
    SendMessage(m_hLblDesc, WM_SETFONT, (WPARAM)hF, TRUE); y += 38;

    // --- CUSTOM GRAMMAR ---
    m_hBtnGram = CreateWindowA("BUTTON", "+ Custom grammar...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, y, W, 26, p, (HMENU)(INT_PTR)ID_BTN_GRM, hi, NULL);
    SendMessage(m_hBtnGram, WM_SETFONT, (WPARAM)hF, TRUE); y += 34;

    // --- ITERATIONS ---
    ctl = CreateWindowA("STATIC", "=== ITERATIONS ===",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 18, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE); y += 20;

    m_hLblIter = CreateWindowA("STATIC", "Iterations: 4",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 18, p, NULL, hi, NULL);
    SendMessage(m_hLblIter, WM_SETFONT, (WPARAM)hF, TRUE); y += 18;

    // Slider (1-SLIDER_MAX) for quick access
    m_hSlIter = CreateWindowA(TRACKBAR_CLASSA, "",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS | TBS_TOOLTIPS,
        10, y, W - 44, 28, p, (HMENU)(INT_PTR)ID_SL_ITER, hi, NULL);
    SendMessage(m_hSlIter, TBM_SETRANGE, TRUE, MAKELPARAM(1, SLIDER_MAX));
    SendMessage(m_hSlIter, TBM_SETPOS, TRUE, 4);

    // Edit box next to slider for typing any value
    m_hEditIter = CreateWindowA("EDIT", "4",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_CENTER,
        W - 30, y + 2, 34, 24, p, (HMENU)(INT_PTR)ID_EDIT_ITER, hi, NULL);
    SendMessage(m_hEditIter, WM_SETFONT, (WPARAM)hF, TRUE);

    // Subclass the edit so we receive VK_RETURN directly inside it.
    // SetWindowLongPtr stores 'this' so editIterProc can call applyIterEdit().
    SetWindowLongPtrA(m_hEditIter, GWLP_USERDATA, (LONG_PTR)this);
    m_editIterOldProc = (WNDPROC)SetWindowLongPtrA(
        m_hEditIter, GWLP_WNDPROC, (LONG_PTR)editIterProc);
    y += 30;

    ctl = CreateWindowA("STATIC", "Slider: 1-20   Edit: any value (Enter)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 16, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE); y += 22;

    ctl = CreateWindowA("STATIC", "WARNING: high values may take time",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 16, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE); y += 22;

    // --- ANGLE ---
    ctl = CreateWindowA("STATIC", "=== BRANCH ANGLE ===",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 18, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE); y += 20;

    m_hLblAngle = CreateWindowA("STATIC", "Angle: (preset)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 18, p, NULL, hi, NULL);
    SendMessage(m_hLblAngle, WM_SETFONT, (WPARAM)hF, TRUE); y += 18;

    m_hSlAngle = CreateWindowA(TRACKBAR_CLASSA, "",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS | TBS_TOOLTIPS,
        10, y, W, 28, p, (HMENU)(INT_PTR)ID_SL_ANG, hi, NULL);
    SendMessage(m_hSlAngle, TBM_SETRANGE, TRUE, MAKELPARAM(0, 89));
    SendMessage(m_hSlAngle, TBM_SETPOS, TRUE, 0);
    y += 28;

    ctl = CreateWindowA("STATIC", "0 = use preset angle",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 16, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE); y += 22;

    // --- COLOURS ---
    ctl = CreateWindowA("STATIC", "=== COLOURS ===",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 18, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE); y += 20;

    m_hChkCol = CreateWindowA("BUTTON", "Custom colours",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        10, y, W, 22, p, (HMENU)(INT_PTR)ID_CHK_COL, hi, NULL);
    SendMessage(m_hChkCol, WM_SETFONT, (WPARAM)hF, TRUE); y += 28;

    ctl = CreateWindowA("STATIC", "Trunk:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, 42, 18, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE);
    m_hSwTrunk = CreateWindowA("SwCls6", "", WS_CHILD | WS_VISIBLE,
        56, y, 34, 18, p, (HMENU)(INT_PTR)ID_SW_TR, hi, NULL);
    m_hBtnTrunk = CreateWindowA("BUTTON", "Change trunk colour",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        96, y, W - 88, 22, p, (HMENU)(INT_PTR)ID_BTN_TR, hi, NULL);
    SendMessage(m_hBtnTrunk, WM_SETFONT, (WPARAM)hF, TRUE); y += 28;

    ctl = CreateWindowA("STATIC", "Leaf:",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, 38, 18, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE);
    m_hSwLeaf = CreateWindowA("SwCls6", "", WS_CHILD | WS_VISIBLE,
        52, y, 34, 18, p, (HMENU)(INT_PTR)ID_SW_LF, hi, NULL);
    m_hBtnLeaf = CreateWindowA("BUTTON", "Change leaf colour",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        96, y, W - 88, 22, p, (HMENU)(INT_PTR)ID_BTN_LF, hi, NULL);
    SendMessage(m_hBtnLeaf, WM_SETFONT, (WPARAM)hF, TRUE); y += 28;

    m_hBtnRstCl = CreateWindowA("BUTTON", "Reset colours",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, y, W, 26, p, (HMENU)(INT_PTR)ID_BTN_RST, hi, NULL);
    SendMessage(m_hBtnRstCl, WM_SETFONT, (WPARAM)hF, TRUE); y += 32;

    // --- CAMERA ---
    ctl = CreateWindowA("STATIC", "=== CAMERA ===",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 18, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE); y += 20;

    ctl = CreateWindowA("STATIC", "LMB drag = rotate   Scroll = zoom",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 18, p, NULL, hi, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)hF, TRUE); y += 20;

    m_hBtnCam = CreateWindowA("BUTTON", "Reset camera",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, y, W, 26, p, (HMENU)(INT_PTR)ID_BTN_CAM, hi, NULL);
    SendMessage(m_hBtnCam, WM_SETFONT, (WPARAM)hF, TRUE); y += 32;

    m_hLblSegs = CreateWindowA("STATIC", "Segments: 0",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 10, y, W, 18, p, NULL, hi, NULL);
    SendMessage(m_hLblSegs, WM_SETFONT, (WPARAM)hF, TRUE);
}

void AppWindow::rebuild()
{
    auto& s = *m_state;
    s.tp.angleOverride = s.angleOver;
    s.tp.useCustomColors = (SendMessage(m_hChkCol, BM_GETCHECK, 0, 0) == BST_CHECKED);
    if (s.tp.useCustomColors) {
        s.tp.trunkR = GetRValue(s.crTrunk) / 255.f;
        s.tp.trunkG = GetGValue(s.crTrunk) / 255.f;
        s.tp.trunkB = GetBValue(s.crTrunk) / 255.f;
        s.tp.leafR = GetRValue(s.crLeaf) / 255.f;
        s.tp.leafG = GetGValue(s.crLeaf) / 255.f;
        s.tp.leafB = GetBValue(s.crLeaf) / 255.f;
    }
    std::string str = expandLSystem(s.current, s.iter);
    s.segments = buildGeometry(s.current, str, s.tp);
    s.dirty = false;
    char buf[80];
    snprintf(buf, sizeof(buf), "Segments: %zu", s.segments.size());
    SetWindowTextA(m_hLblSegs, buf);
}

void AppWindow::updateLabels()
{
    auto& s = *m_state;
    char buf[128];

    snprintf(buf, sizeof(buf), "Iterations: %d", s.iter);
    SetWindowTextA(m_hLblIter, buf);

    if (m_hSlIter) {
        int slPos = s.iter;
        if (slPos < 1) slPos = 1;
        if (slPos > SLIDER_MAX) slPos = SLIDER_MAX;
        SendMessage(m_hSlIter, TBM_SETPOS, TRUE, slPos);
    }

    if (m_hEditIter) {
        snprintf(buf, sizeof(buf), "%d", s.iter);
        SetWindowTextA(m_hEditIter, buf);
    }

    float ang = (s.angleOver > 0.f) ? s.angleOver : s.current.angle;
    snprintf(buf, sizeof(buf), "Angle: %.1f%s", ang,
        (s.angleOver > 0.f) ? " (custom)" : "");
    SetWindowTextA(m_hLblAngle, buf);

    std::string desc = s.current.description;
    if (desc.size() > 58) desc = desc.substr(0, 55) + "...";
    SetWindowTextA(m_hLblDesc, desc.c_str());
}

void AppWindow::applyIterEdit()
{
    if (!m_hEditIter) return;
    char buf[32] = {};
    GetWindowTextA(m_hEditIter, buf, sizeof(buf));
    int v = atoi(buf);
    if (v < 1)  v = 1;
    if (v > 99) v = 99;
    m_state->iter = v;
    m_state->dirty = true;
    updateLabels();
    InvalidateRect(m_hwndGL, NULL, FALSE);
}

void AppWindow::syncSwatches()
{
    auto& s = *m_state;
    s.crTrunk = RGB((int)(s.current.colTrunk.r * 255),
        (int)(s.current.colTrunk.g * 255),
        (int)(s.current.colTrunk.b * 255));
    s.crLeaf = RGB((int)(s.current.colLeaf.r * 255),
        (int)(s.current.colLeaf.g * 255),
        (int)(s.current.colLeaf.b * 255));
    if (m_hSwTrunk) InvalidateRect(m_hSwTrunk, NULL, FALSE);
    if (m_hSwLeaf)  InvalidateRect(m_hSwLeaf, NULL, FALSE);
}

bool AppWindow::pickColor(HWND parent, COLORREF& col)
{
    CHOOSECOLORA cc = {};
    cc.lStructSize = sizeof(cc); cc.hwndOwner = parent;
    cc.rgbResult = col; cc.lpCustColors = m_custColors;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColorA(&cc)) { col = cc.rgbResult; return true; }
    return false;
}

void AppWindow::loadCustomGrammar(const LSystem& ls)
{
    m_state->current = ls;
    m_state->angleOver = 0.f;
    m_state->dirty = true;
    SendMessage(m_hSlAngle, TBM_SETPOS, TRUE, 0);
    updateLabels();
    InvalidateRect(m_hwndGL, NULL, FALSE);
}

LRESULT AppWindow::onMainMsg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_SIZE: {
        int W = LOWORD(lp), H = HIWORD(lp);
        if (W < PANEL_W + 100) W = PANEL_W + 100;
        if (m_hwndPanel) SetWindowPos(m_hwndPanel, NULL, 0, 0, PANEL_W, H, SWP_NOZORDER);
        if (m_hwndGL)    SetWindowPos(m_hwndGL, NULL, PANEL_W, 0, W - PANEL_W, H, SWP_NOZORDER);
        return 0;
    }
    case WM_GETMINMAXINFO: {
        auto* mm = (MINMAXINFO*)lp;
        mm->ptMinTrackSize.x = PANEL_W + 300;
        mm->ptMinTrackSize.y = 400;
        return 0;
    }
    case WM_DESTROY:
        if (m_hRC) { wglMakeCurrent(NULL, NULL); wglDeleteContext(m_hRC); }
        if (m_hDC) ReleaseDC(m_hwndGL, m_hDC);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT AppWindow::onGLMsg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto& s = *m_state;
    switch (msg) {
    case WM_SIZE:
        s.rs.winW = LOWORD(lp); s.rs.winH = HIWORD(lp); return 0;
    case WM_PAINT: {
        if (s.dirty) rebuild();
        wglMakeCurrent(m_hDC, m_hRC);
        renderScene(s.segments, s.rs);
        SwapBuffers(m_hDC);
        ValidateRect(hwnd, NULL);
        return 0;
    }
    case WM_LBUTTONDOWN:
        m_mouseDown = true;
        m_lastMX = GET_X_LPARAM(lp); m_lastMY = GET_Y_LPARAM(lp);
        SetCapture(hwnd); return 0;
    case WM_LBUTTONUP:
        m_mouseDown = false; ReleaseCapture(); return 0;
    case WM_MOUSEMOVE:
        if (m_mouseDown) {
            int mx = GET_X_LPARAM(lp), my = GET_Y_LPARAM(lp);
            s.rs.camRotY += (mx - m_lastMX) * .5f;
            s.rs.camRotX += (my - m_lastMY) * .5f;
            m_lastMX = mx; m_lastMY = my;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    case WM_MOUSEWHEEL: {
        float d = (GET_WHEEL_DELTA_WPARAM(wp) > 0) ? 1.10f : 1.f / 1.10f;
        s.rs.camZoom = std::max(0.01f, s.rs.camZoom * d);
        InvalidateRect(hwnd, NULL, FALSE); return 0;
    }
    case WM_ERASEBKGND: return 1;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT AppWindow::onPanelMsg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto& s = *m_state;

    switch (msg) {
        // Dark background for labels
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wp;
        SetBkColor(hdc, RGB(24, 24, 32));
        SetTextColor(hdc, RGB(200, 215, 240));
        static HBRUSH hbr = CreateSolidBrush(RGB(24, 24, 32));
        return (LRESULT)hbr;
    }
    case WM_CTLCOLOREDIT: {
        // Edit box for iterations — dark bg too
        HDC hdc = (HDC)wp;
        SetBkColor(hdc, RGB(40, 42, 58));
        SetTextColor(hdc, RGB(220, 230, 255));
        static HBRUSH hbrE = CreateSolidBrush(RGB(40, 42, 58));
        return (LRESULT)hbrE;
    }
    case WM_CTLCOLORBTN:
        return (LRESULT)GetStockObject(NULL_BRUSH);

    case WM_COMMAND: {
        int id = LOWORD(wp);
        int notify = HIWORD(wp);

        if (id == ID_COMBO && notify == CBN_SELCHANGE) {
            s.presetIdx = (int)SendMessage(m_hCombo, CB_GETCURSEL, 0, 0);
            s.current = s.presets[s.presetIdx];
            s.angleOver = 0.f;
            s.dirty = true;
            SendMessage(m_hSlAngle, TBM_SETPOS, TRUE, 0);
            updateLabels();
            syncSwatches();
            InvalidateRect(m_hwndGL, NULL, FALSE);
        }

        // Edit box: apply on focus loss (covers clicking away with mouse)
        if (id == ID_EDIT_ITER && notify == EN_KILLFOCUS)
            applyIterEdit();

        if (id == ID_BTN_TR) {
            if (pickColor(hwnd, s.crTrunk)) {
                InvalidateRect(m_hSwTrunk, NULL, FALSE);
                if (SendMessage(m_hChkCol, BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    s.dirty = true; InvalidateRect(m_hwndGL, NULL, FALSE);
                }
            }
        }
        if (id == ID_BTN_LF) {
            if (pickColor(hwnd, s.crLeaf)) {
                InvalidateRect(m_hSwLeaf, NULL, FALSE);
                if (SendMessage(m_hChkCol, BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    s.dirty = true; InvalidateRect(m_hwndGL, NULL, FALSE);
                }
            }
        }
        if (id == ID_CHK_COL) { s.dirty = true; InvalidateRect(m_hwndGL, NULL, FALSE); }
        if (id == ID_BTN_CAM) {
            s.rs.camRotX = 20.f; s.rs.camRotY = 30.f; s.rs.camZoom = 1.f;
            InvalidateRect(m_hwndGL, NULL, FALSE);
        }
        if (id == ID_BTN_RST) {
            syncSwatches();
            if (SendMessage(m_hChkCol, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                s.dirty = true; InvalidateRect(m_hwndGL, NULL, FALSE);
            }
        }
        if (id == ID_BTN_GRM) {
            extern void openGrammarDialog(HWND parent, AppWindow * win);
            openGrammarDialog(m_hwndMain, this);
        }
        break;
    }

    case WM_HSCROLL: {
        HWND hs = (HWND)lp;
        int pos = (int)SendMessage(hs, TBM_GETPOS, 0, 0);
        if (hs == m_hSlIter) {
            s.iter = pos;
            s.dirty = true;
            // sync edit box
            char buf[16]; snprintf(buf, sizeof(buf), "%d", pos);
            SetWindowTextA(m_hEditIter, buf);
        }
        else if (hs == m_hSlAngle) {
            s.angleOver = (pos == 0) ? 0.f : (float)pos;
            s.dirty = true;
        }
        updateLabels();
        InvalidateRect(m_hwndGL, NULL, FALSE);
        break;
    }
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}