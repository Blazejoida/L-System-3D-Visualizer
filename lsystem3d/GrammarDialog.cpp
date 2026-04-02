#include "GrammarDialog.h"
#include <commctrl.h>
#include <cstdio>
#include <sstream>
#include <vector>

//  Help text (right pane) — ASCII only
static const char* HELP_TXT =
"TURTLE SYMBOLS:\r\n"
"  F A..Z  step forward (draw)\r\n"
"  f       step forward (no draw)\r\n"
"  + -     yaw left / right\r\n"
"  & ^     pitch down / up\r\n"
"  < >     roll left / right\r\n"
"  |       U-turn 180 deg\r\n"
"  [ ]     push / pop state\r\n"
"\r\n"
"RULE FORMAT:\r\n"
"  One rule per line: X=...\r\n"
"  Lines starting with # are ignored.\r\n"
"\r\n"
"--- EXAMPLES ---\r\n"
"\r\n"
"[1] Koch curve\r\n"
"  Axiom : F++F++F\r\n"
"  Rules : F=F+F--F+F\r\n"
"  Angle : 60   Scale: 1.0\r\n"
"  Iter  : 3-5\r\n"
"\r\n"
"[2] Classic tree\r\n"
"  Axiom : F\r\n"
"  Rules : F=FF-[-F+F+F]+[+F-F-F]\r\n"
"  Angle : 22.5  Scale: 0.5\r\n"
"  Iter  : 4-6\r\n"
"\r\n"
"[3] Dragon curve\r\n"
"  Axiom : FX\r\n"
"  Rules : X=X+YF+\r\n"
"          Y=-FX-Y\r\n"
"  Angle : 90   Scale: 1.0\r\n"
"  Iter  : 10-12\r\n"
"\r\n"
"[4] Barnsley fern\r\n"
"  Axiom : X\r\n"
"  Rules : X=F+[[X]-X]-F[-FX]+X\r\n"
"          F=FF\r\n"
"  Angle : 25   Scale: 0.6\r\n"
"  Iter  : 5-7\r\n"
"\r\n"
"[5] 3D bush\r\n"
"  Axiom : X\r\n"
"  Rules : X=F[&+X][^-X][>+X][<-X]F\r\n"
"  Angle : 26   Scale: 0.58\r\n"
"  Iter  : 3-5\r\n"
"\r\n"
"TIPS:\r\n"
"  Angle 20-30  -> natural trees\r\n"
"  Angle 60/90  -> geometric fractals\r\n"
"  Scale 0.5    -> short branches\r\n"
"  Scale 1.0    -> equal segments\r\n";


GrammarDialog::GrammarDialog()  {}
GrammarDialog::~GrammarDialog() {}


LRESULT CALLBACK GrammarDialog::wndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    if (msg == WM_CREATE) {
        auto cs = (CREATESTRUCTA*)lp;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        auto* self = (GrammarDialog*)cs->lpCreateParams;
        self->m_hwnd = hwnd;
        self->createControls();
        // Initial layout with creation size
        RECT rc; GetClientRect(hwnd, &rc);
        self->layoutControls(rc.right, rc.bottom);
        return 0;
    }
    auto* self = (GrammarDialog*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    if (!self) return DefWindowProcA(hwnd,msg,wp,lp);
    return self->handleMsg(hwnd,msg,wp,lp);
}

void GrammarDialog::show(HWND parent, ApplyFn cb)
{
    m_parent = parent;
    m_cb     = cb;

    if (m_hwnd && IsWindow(m_hwnd)) {
        ShowWindow(m_hwnd, SW_SHOW);
        SetForegroundWindow(m_hwnd);
        return;
    }

    HINSTANCE hi = GetModuleHandleA(NULL);

    // Register class
    WNDCLASSA wc     = {};
    wc.lpfnWndProc   = wndProc;
    wc.hInstance     = hi;
    wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "GramDlg6";
    wc.hIcon         = LoadIconA(NULL, IDI_APPLICATION);
    RegisterClassA(&wc);

    RECT pr; GetWindowRect(parent, &pr);

    CreateWindowExA(0, "GramDlg6",
        "L-System Grammar Editor",
        WS_OVERLAPPEDWINDOW,
        pr.left + 60, pr.top + 60, 860, 680,
        parent, NULL, hi, (LPVOID)this);
    // m_hwnd set inside WM_CREATE

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
}

void GrammarDialog::createControls()
{
    HINSTANCE hi = GetModuleHandleA(NULL);
    HFONT hF = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    m_hMono = CreateFontA(14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
                          DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
                          FIXED_PITCH|FF_MODERN,"Consolas");
    if (!m_hMono) m_hMono = hF;

    HWND w = m_hwnd;  // parent

    // Static labels
    auto mkLbl = [&](const char* t) -> HWND {
        HWND h = CreateWindowA("STATIC", t,
            WS_CHILD|WS_VISIBLE|SS_LEFT,
            0,0,10,10, w, NULL, hi, NULL);
        SendMessage(h, WM_SETFONT, (WPARAM)hF, TRUE);
        return h;
    };

    m_lblName  = mkLbl("Name (optional):");
    m_lblAxiom = mkLbl("Axiom:");
    m_lblRules = mkLbl("Rules  (one per line,  format: X=...):");
    m_lblAngle = mkLbl("Angle (degrees, e.g. 22.5):");
    m_lblScale = mkLbl("Branch scale (e.g. 0.5):");

    // Edit controls
    DWORD singleSty = WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL;
    DWORD multiSty  = WS_CHILD|WS_VISIBLE|WS_BORDER|WS_VSCROLL|
                      ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN;

    m_hName = CreateWindowA("EDIT","My L-system",
        singleSty, 0,0,10,10, w, (HMENU)101, hi, NULL);
    SendMessage(m_hName, WM_SETFONT, (WPARAM)hF, TRUE);

    m_hAxiom = CreateWindowA("EDIT","F",
        singleSty, 0,0,10,10, w, (HMENU)102, hi, NULL);
    SendMessage(m_hAxiom, WM_SETFONT, (WPARAM)m_hMono, TRUE);

    m_hRules = CreateWindowA("EDIT","F=FF-[-F+F+F]+[+F-F-F]",
        multiSty, 0,0,10,10, w, (HMENU)103, hi, NULL);
    SendMessage(m_hRules, WM_SETFONT, (WPARAM)m_hMono, TRUE);

    m_hAngle = CreateWindowA("EDIT","22.5",
        singleSty, 0,0,10,10, w, (HMENU)104, hi, NULL);
    SendMessage(m_hAngle, WM_SETFONT, (WPARAM)hF, TRUE);

    m_hScale = CreateWindowA("EDIT","0.5",
        singleSty, 0,0,10,10, w, (HMENU)105, hi, NULL);
    SendMessage(m_hScale, WM_SETFONT, (WPARAM)hF, TRUE);

    // Buttons 
    m_hApply = CreateWindowA("BUTTON","Apply  (F5)",
        WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,
        0,0,10,10, w, (HMENU)1001, hi, NULL);
    SendMessage(m_hApply, WM_SETFONT, (WPARAM)hF, TRUE);

    m_hClose = CreateWindowA("BUTTON","Close",
        WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
        0,0,10,10, w, (HMENU)1002, hi, NULL);
    SendMessage(m_hClose, WM_SETFONT, (WPARAM)hF, TRUE);

    // Status bar 
    m_hStatus = CreateWindowA("STATIC","Ready.",
        WS_CHILD|WS_VISIBLE|SS_LEFT|WS_BORDER,
        0,0,10,10, w, NULL, hi, NULL);
    SendMessage(m_hStatus, WM_SETFONT, (WPARAM)hF, TRUE);

    // Help (right pane, read-only multiline edit)
    m_hHelp = CreateWindowA("EDIT", HELP_TXT,
        WS_CHILD|WS_VISIBLE|WS_BORDER|WS_VSCROLL|
        ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL,
        0,0,10,10, w, (HMENU)106, hi, NULL);
    SendMessage(m_hHelp, WM_SETFONT, (WPARAM)m_hMono, TRUE);
}

void GrammarDialog::layoutControls(int W, int H)
{
    if (!m_hName) return;           // not yet created

    int margin  = 10;
    int leftW   = (W - margin*3) / 2;   // left pane width
    int rightX  = margin + leftW + margin;
    int rightW  = W - rightX - margin;

    // Row heights
    int lblH  = 18;
    int editH = 24;
    int rulesH = H - 310;            // rules box gets remaining height
    if (rulesH < 60) rulesH = 60;

    auto mv = [](HWND h, int x, int y, int w, int hh){
        SetWindowPos(h, NULL, x, y, w, hh, SWP_NOZORDER|SWP_NOACTIVATE);
    };

    int y = margin;

    mv(m_lblName,  margin, y,        leftW, lblH);  y += lblH + 2;
    mv(m_hName,    margin, y,        leftW, editH); y += editH + 8;

    mv(m_lblAxiom, margin, y,        leftW, lblH);  y += lblH + 2;
    mv(m_hAxiom,   margin, y,        leftW, editH); y += editH + 8;

    mv(m_lblRules, margin, y,        leftW, lblH);  y += lblH + 2;
    mv(m_hRules,   margin, y,        leftW, rulesH); y += rulesH + 10;

    mv(m_lblAngle, margin, y,        leftW, lblH);  y += lblH + 2;
    mv(m_hAngle,   margin, y,        120,   editH); y += editH + 8;

    mv(m_lblScale, margin, y,        leftW, lblH);  y += lblH + 2;
    mv(m_hScale,   margin, y,        120,   editH); y += editH + 12;

    int btnW = (leftW - 8) / 2;
    mv(m_hApply,   margin,            y, btnW, 32);
    mv(m_hClose,   margin + btnW + 8, y, btnW, 32);
    y += 40;

    mv(m_hStatus,  margin, y, leftW, 40);

    // Right pane: help text fills full height
    mv(m_hHelp, rightX, margin, rightW, H - margin*2);
}

LRESULT GrammarDialog::handleMsg(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    switch (msg)
    {
    case WM_SIZE:
        layoutControls(LOWORD(lp), HIWORD(lp));
        return 0;

    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id == 1001)                          { onApply();                    return 0; }
        if (id == 1002)                          { ShowWindow(hwnd,SW_HIDE);     return 0; }
        break;
    }

    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) { ShowWindow(hwnd,SW_HIDE); return 0; }
        if (wp == VK_F5)     { onApply();                return 0; }
        break;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    }
    return DefWindowProcA(hwnd,msg,wp,lp);
}

bool GrammarDialog::parseAndBuild(LSystem& out, std::string& err)
{
    char buf[512];

    GetWindowTextA(m_hAxiom, buf, sizeof(buf));
    out.axiom = buf;
    if (out.axiom.empty()) { err = "Axiom cannot be empty."; return false; }

    GetWindowTextA(m_hName, buf, sizeof(buf));
    out.name = buf[0] ? std::string(buf) : "Custom";

    // Rules multiline
    int len = GetWindowTextLengthA(m_hRules) + 2;
    std::vector<char> rbuf(len + 1, 0);
    GetWindowTextA(m_hRules, rbuf.data(), len);

    out.rules.clear();
    std::istringstream ss(rbuf.data());
    std::string line;
    int lineNo = 0;
    while (std::getline(ss, line)) {
        ++lineNo;
        if (!line.empty() && line.back()=='\r') line.pop_back();
        if (line.empty() || line[0]=='#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) {
            char tmp[128];
            snprintf(tmp,sizeof(tmp),"Line %d: missing '=': %s",lineNo,line.c_str());
            err = tmp; return false;
        }
        std::string lhs = line.substr(0, eq);
        std::string rhs = line.substr(eq + 1);
        while (!lhs.empty() && (lhs[0]==' '||lhs[0]=='\t')) lhs.erase(0,1);
        while (!lhs.empty() && (lhs.back()==' '||lhs.back()=='\t')) lhs.pop_back();
        if (lhs.size() != 1) {
            err = "Left side must be one character: " + lhs; return false;
        }
        out.rules[lhs[0]] = rhs;
    }

    // Angle
    GetWindowTextA(m_hAngle, buf, sizeof(buf));
    float ang = (float)atof(buf);
    if (ang < 0.5f || ang > 360.f) {
        err = "Angle must be 0.5 - 360."; return false;
    }
    out.angle = ang;

    // Scale
    GetWindowTextA(m_hScale, buf, sizeof(buf));
    float sc = (float)atof(buf);
    if (sc < 0.1f || sc > 2.0f) {
        err = "Scale must be 0.1 - 2.0."; return false;
    }
    out.stepScale   = sc;
    out.stepLen     = 1.0f;
    out.colTrunk    = {0.38f,0.22f,0.06f};
    out.colBranch   = {0.20f,0.52f,0.08f};
    out.colLeaf     = {0.16f,0.70f,0.10f};
    out.description = "Custom grammar.";
    return true;
}

void GrammarDialog::onApply()
{
    LSystem ls;
    std::string err;
    if (!parseAndBuild(ls, err)) {
        SetWindowTextA(m_hStatus, ("ERROR: " + err).c_str());
        return;
    }
    char info[128];
    snprintf(info,sizeof(info),"OK  axiom:%zu  rules:%zu",
             ls.axiom.size(), ls.rules.size());
    SetWindowTextA(m_hStatus, info);
    if (m_cb) m_cb(ls);
}
