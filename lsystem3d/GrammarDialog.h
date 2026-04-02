#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "LSystem.h"
#include <functional>
#include <string>

// Modeless grammar editor window.
// Controls are created once in WM_CREATE (not in show()).
// WM_SIZE repositions them. Window hides on close.
class GrammarDialog
{
public:
    using ApplyFn = std::function<void(const LSystem&)>;

    GrammarDialog();
    ~GrammarDialog();

    // Open or raise the window.
    void show(HWND parent, ApplyFn cb);

    HWND hwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK wndProc(HWND,UINT,WPARAM,LPARAM);
    LRESULT handleMsg(HWND,UINT,WPARAM,LPARAM);

    void createControls();            // called from WM_CREATE
    void layoutControls(int W, int H);// called from WM_SIZE and WM_CREATE
    void onApply();
    bool parseAndBuild(LSystem& out, std::string& err);

    HWND     m_hwnd   = NULL;
    HWND     m_parent = NULL;
    ApplyFn  m_cb;
    HFONT    m_hMono  = NULL;

    // Edit controls 
    HWND m_hName   = NULL;
    HWND m_hAxiom  = NULL;
    HWND m_hRules  = NULL;
    HWND m_hAngle  = NULL;
    HWND m_hScale  = NULL;
    HWND m_hApply  = NULL;
    HWND m_hClose  = NULL;
    HWND m_hStatus = NULL;
    HWND m_hHelp   = NULL;

    // Static labels
    HWND m_lblName  = NULL;
    HWND m_lblAxiom = NULL;
    HWND m_lblRules = NULL;
    HWND m_lblAngle = NULL;
    HWND m_lblScale = NULL;
};
