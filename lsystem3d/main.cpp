// L-System 3D Visualizer  v6
// main.cpp -- WinMain entry point only.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>

#include "LSystem.h"
#include "AppWindow.h"
#include "GrammarDialog.h"

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"comctl32.lib")


static AppState      g_state;
static AppWindow     g_appWindow;
static GrammarDialog g_grammarDlg;

void openGrammarDialog(HWND parent, AppWindow* win)
{
    g_grammarDlg.show(parent, [win](const LSystem& ls) {
        win->loadCustomGrammar(ls);
    });
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow)
{
    INITCOMMONCONTROLSEX icx = { sizeof(icx), ICC_BAR_CLASSES };
    InitCommonControlsEx(&icx);

    g_state.presets = buildPresets();
    g_state.current = g_state.presets[0];

    if (!g_appWindow.create(hInst, g_state, nShow))
        return 1;

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        HWND gdHwnd = g_grammarDlg.hwnd();
        if (gdHwnd && IsWindow(gdHwnd) && IsWindowVisible(gdHwnd) &&
            IsDialogMessageA(gdHwnd, &msg))
            continue;
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
