#include <MainWindow.hpp>

#include <resource.h>
#include <commctrl.h>
#include <windowsx.h>

#define LEFT(x, y, cx, cy) x, y, cx, cy
#define RIGHT(x, y, cx, cy) (x - cx), y, cx, cy
#define BOTTOM(x, y, cx, cy) x, (y - cy), cx, cy
#define BOTTOMRIGHT(x, y, cx, cy) (x - cx), (y - cy), cx, cy
#define KEYBOARD_IMAGE      0xAA00
#define KEYBOARD_VOLUME     0xAA01
#define KEYBOARD_FORCE      0xAA02
#define KEYBOARD_INSTRUMENT 0xAA03
#define MIDI_MESSAGE(handle, code, arg1, arg2) \
    midiOutShortMsg(handle, ((arg2 & 0x7F) << 16) |\
                    ((arg1 & 0x7F) << 8) | (code & 0xFF))

#pragma comment(lib, "winmm.lib")

DWORD rgbWindowBackground;

static char keymap[256];

BOOL MainWindow::WinRegisterClass(WNDCLASS *pwc)
{
    pwc->style = CS_HREDRAW | CS_VREDRAW;
    return __super::WinRegisterClass(pwc);
}

BOOL EnableCloseButton(const HWND hwnd, const BOOL bState)
{
    HMENU hMenu;
    UINT dwExtra;

    if (hwnd == NULL)
        return FALSE;
    if ((hMenu = GetSystemMenu(hwnd, FALSE)) == NULL)
        return FALSE;
    dwExtra = bState ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
    return EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | dwExtra) != -1;
}

LRESULT MainWindow::OnCreate()
{
    NONCLIENTMETRICS ncmMetrics = { sizeof(NONCLIENTMETRICS) };
    RECT client;

    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncmMetrics, 0);
    GetClientRect(m_hwnd, &client);

    hFont = CreateFontIndirect(&ncmMetrics.lfMessageFont);
    rgbWindowBackground = GetSysColor(COLOR_WINDOW);
    hBrush = GetSysColorBrush(COLOR_WINDOW);
    hKeyboardLayout = (HBITMAP) LoadImage(GetInstance(), MAKEINTRESOURCE(RID_KEYBOARD),
                                          IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT);
    hdcKeyboard = CreateCompatibleDC(GetDC(m_hwnd));
    SelectBitmap(hdcKeyboard, hKeyboardLayout);

    // Children
    m_volumeLabel = CreateWindow(WC_STATIC, L"Volume:",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, 0, 0, 0,
            m_hwnd, NULL, GetInstance(), NULL);
    m_forceLabel = CreateWindow(WC_STATIC, L"Force:",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, 0, 0, 0,
            m_hwnd, NULL, GetInstance(), NULL);
    m_instruLabel = CreateWindow(WC_STATIC, L"Instrument:",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, 0, 0, 0,
            m_hwnd, NULL, GetInstance(), NULL);

    m_volumeBar = CreateWindow(TRACKBAR_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | TBS_NOTICKS, 0, 0, 0, 0,
            m_hwnd, (HMENU) KEYBOARD_VOLUME, GetInstance(), NULL);
    m_forceBar = CreateWindow(TRACKBAR_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | TBS_NOTICKS, 0, 0, 0, 0,
            m_hwnd, (HMENU) KEYBOARD_FORCE, GetInstance(), NULL);
    m_instruSelect = CreateWindow(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE |
                    CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL, 0, 0, 0, 0,
            m_hwnd, (HMENU) KEYBOARD_INSTRUMENT, GetInstance(), NULL);

    SendMessage(m_volumeBar, TBM_SETRANGEMIN, FALSE, 0x0000);
    SendMessage(m_volumeBar, TBM_SETRANGEMAX, FALSE, 0xFFFF);
    SendMessage(m_forceBar, TBM_SETRANGE, FALSE, 127 << 16);
    SendMessage(m_volumeBar, TBM_SETPOS, FALSE, 0xFFFF);
    SendMessage(m_forceBar, TBM_SETPOS, FALSE, 64);
    m_force = 64;
    m_volume = 0xFFFF;

    WCHAR buf[MAX_PATH];
    int piano;
    SendMessage(m_instruSelect, CB_INITSTORAGE, 128, 4096);
    for (int i = 0; i < 128; ++i) {
        int id;
        LoadString(GetInstance(), 0xAE00 | i, buf, MAX_PATH);
        id = SendMessage(m_instruSelect, CB_ADDSTRING, 0, (LPARAM) buf);
        SendMessage(m_instruSelect, CB_SETITEMDATA, id, i);
    }
    LoadString(GetInstance(), 0xAE00, buf, MAX_PATH);
    piano = SendMessage(m_instruSelect, CB_FINDSTRING, 0, (LPARAM) buf);
    SendMessage(m_instruSelect, CB_SETCURSEL, piano, 0);
#define SETFONT(hwnd) PostMessage(hwnd, WM_SETFONT, (WPARAM) hFont, (LPARAM) TRUE)
    SETFONT(m_volumeLabel);
    SETFONT(m_volumeBar);
    SETFONT(m_forceLabel);
    SETFONT(m_forceBar);
    SETFONT(m_instruLabel);
    SETFONT(m_instruSelect);
#undef SETFONT

    if (midiOutOpen(&m_midi, 0, NULL, NULL, CALLBACK_NULL) != MMSYSERR_NOERROR)
        MessageBox(m_hwnd, L"Failed to open MIDI device!", L"Fatal Error", MB_ICONERROR);

    keymap[VK_OEM_3]  = 54; // `~ key
    keymap['Q']  = 55;
    keymap[VK_TAB] = 55;
    keymap['A']  = 57;
    keymap['Z']  = 57;
    keymap['W']  = 56;
    keymap['E']  = 58;
    keymap['S']  = 59;
    keymap['X']  = 59;
    keymap['D']  = 60;
    keymap['C']  = 60;
    keymap['R']  = 61;
    keymap['F']  = 62;
    keymap['V']  = 62;
    keymap['T']  = 63;
    keymap['G']  = 64;
    keymap['B']  = 64;
    keymap['H']  = 65;
    keymap['N']  = 65;
    keymap['U']  = 66;
    keymap['J']  = 67;
    keymap['M']  = 67;
    keymap['I']  = 68;
    keymap['K']  = 69;
    keymap[VK_OEM_COMMA]  = 69;
    keymap['O']  = 70;
    keymap['L']  = 71;
    keymap[VK_OEM_PERIOD]  = 71;
    keymap[VK_OEM_1]  = 72; // ;:
    keymap[VK_OEM_2]  = 72; // /?
    keymap[VK_OEM_7] = 74; // '"
    keymap[VK_OEM_4]  = 73; // [
    keymap[VK_OEM_6]  = 75; // ]
    keymap[VK_RETURN] = 76;
    keymap[VK_OEM_5] = 77; // \|
    keymap[VK_BACK] = 77;
    keymap[0x31]  = 57;
    keymap[0x32]  = 59;
    keymap[0x33]  = 60;
    keymap[0x34]  = 62;
    keymap[0x35]  = 64;
    keymap[0x36]  = 65;
    keymap[0x37]  = 67;
    keymap[0x38]  = 69;
    keymap[0x39]  = 71;
    keymap[0x30]  = 72;
    keymap[VK_OEM_MINUS]  = 74;
    keymap[VK_OEM_PLUS]  = 76;
    PostMessage(m_hwnd, WM_INPUTLANGCHANGE, 0, 0);
    
    SetTimer(m_hwnd, 0xDE00, 1000, NULL);
    return 0;
}

LRESULT MainWindow::OnDestroy()
{
    DestroyWindow(m_volumeLabel);
    DestroyWindow(m_volumeBar);
    DestroyWindow(m_forceLabel);
    DestroyWindow(m_forceBar);
    DestroyWindow(m_instruLabel);
    DestroyWindow(m_instruSelect);
    midiOutClose(m_midi);
    return 0;
}

HICON MainWindow::GetIcon()
{
    return LoadIcon(GetInstance(), MAKEINTRESOURCE(RID_ICON));
}

void MainWindow::PaintContent(PAINTSTRUCT *pps)
{
    BITMAP bm;
    int cx, cy, tx, ty, x, y;
    double ratio;
    RECT client;
    GetClientRect(m_hwnd, &client);
    GetObject(hKeyboardLayout, sizeof(BITMAP), &bm);
    cx = client.right - 24, cy = client.bottom - 117;
    ratio = ((double) bm.bmWidth) / ((double) bm.bmHeight);
    if (cx / ratio > cy)
        tx = (int) (cy * ratio), ty = (int) cy;
    else
        tx = (int) cx, ty = (int) (cx / ratio);
    x = 12 + (cx - tx) / 2;
    y = 12 + (cy - ty) / 2;
    StretchBlt(pps->hdc, x, y, tx, ty, hdcKeyboard, 0, 0, bm.bmWidth, bm.bmHeight, SRCAND);
}

void MainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);
    PaintContent(&ps);
    EndPaint(m_hwnd, &ps);
}

WORD MainWindow::GetQWERTYKeyCode(WORD wKeyCode)
{
    if (isQWERTY)
        return wKeyCode;
    UINT uiScan = MapVirtualKey(wKeyCode, MAPVK_VK_TO_VSC);
    if (!uiScan)
        return wKeyCode;
    UINT uiQWERTY = MapVirtualKeyEx(uiScan, MAPVK_VSC_TO_VK, hklQWERTY);
    if (uiScan)
        return (WORD) uiQWERTY;
    else
        return wKeyCode;
}

WORD MainWindow::GetRealKeyCode(WORD wQWERTYCode)
{
    if (isQWERTY)
        return wQWERTYCode;
    UINT uiScan = MapVirtualKeyEx(wQWERTYCode, MAPVK_VK_TO_VSC, hklQWERTY);
    if (!uiScan)
        return wQWERTYCode;
    UINT uiKeyCode = MapVirtualKey(uiScan, MAPVK_VSC_TO_VK);
    if (uiScan)
        return (WORD) uiKeyCode;
    else
        return wQWERTYCode;
}

int GetMIDINote(WPARAM wCode)
{
    int state = 0;
    int note;
    if (GetKeyState(VK_CONTROL) < 0)
        state |= 0x001;
    if (GetKeyState(VK_SHIFT) < 0)
        state |= 0x010;
    if (GetKeyState(VK_MENU) < 0)
        state |= 0x100;

    note = keymap[wCode];
    switch (state) {
    case 0x001:
        note -= 24;
        break;
    case 0x010:
        note += 24;
        break;
    case 0x100:
        note += 48;
        break;
    }
    return note;
}

bool MainWindow::Play(WPARAM wCode, LPARAM lParam, bool down)
{
    int note;
    wCode = GetQWERTYKeyCode((WORD) wCode);
    if (wCode > 255 || !keymap[wCode] || (down && (lParam & 0x40000000)))
        return false;

    note = GetMIDINote(wCode);
    if (down) {
        int num = note;
        while (num > 24)
            num -= 24;
        while (num < 0x7F) {
            if (num != note)
                MIDI_MESSAGE(m_midi, 0x90, num, 0);
            num += 24;
        }
    }
    if (down)
        MIDI_MESSAGE(m_midi, 0x90, note, m_force);
    else
        MIDI_MESSAGE(m_midi, 0x90, note, 0);
    return true;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        return OnCreate();
    case WM_DESTROY:
        return OnDestroy();
    case WM_NCDESTROY:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        OnPaint();
        return 0;
    case WM_SIZE: {
        RECT client;
        HDWP hdwp;
        GetClientRect(m_hwnd, &client);
#define REPOS(hwnd, k) hdwp = DeferWindowPos(hdwp, hwnd, 0, k, SWP_NOACTIVATE|SWP_NOZORDER)
        hdwp = BeginDeferWindowPos(14);
        REPOS(m_volumeLabel,    BOTTOM(12, client.bottom - 12, 70, 25));
        REPOS(m_volumeBar,      BOTTOM(82, client.bottom - 12, client.right - 94, 25));
        REPOS(m_forceLabel,     BOTTOM(12, client.bottom - 42, 70, 25));
        REPOS(m_forceBar,       BOTTOM(82, client.bottom - 42, client.right - 94, 25));
        REPOS(m_instruLabel,    BOTTOM(12, client.bottom - 72, 70, 25));
        REPOS(m_instruSelect,   BOTTOM(82, client.bottom - 72, client.right - 94, 25));
        EndDeferWindowPos(hdwp);
#undef REPOS
        return 0;
      }
    case WM_COMMAND:
        switch (HIWORD(wParam)) {
        case CBN_SELCHANGE:
            switch (LOWORD(wParam)) {
            case KEYBOARD_INSTRUMENT:
                m_instrument = SendMessage((HWND) lParam, CB_GETITEMDATA,
                        SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0), 0);
                MIDI_MESSAGE(m_midi, 0xC0, m_instrument, 0);
            }
        }
        break;
    case WM_HSCROLL:
        switch (LOWORD(wParam)) {
        case TB_THUMBPOSITION:
            if (lParam == (LPARAM) m_volumeBar) {
                m_volume = HIWORD(wParam);
                midiOutSetVolume(m_midi, m_volume);
            } else if (lParam == (LPARAM) m_forceBar)
                m_force = HIWORD(wParam);
            break;
        }
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (Play(wParam, lParam, true))
            return 0;
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (Play(wParam, lParam, false))
            return 0;
        break;
    case WM_LBUTTONDOWN:
        SetFocus(m_hwnd);
        return 0;
    case WM_MOUSEWHEEL:
        if (GetFocus() != m_forceBar)
            SendMessage(m_forceBar, WM_MOUSEWHEEL,
                -GET_WHEEL_DELTA_WPARAM(wParam) * 2 << 16 |
                        GET_KEYSTATE_WPARAM(wParam), lParam);
    case WM_INPUTLANGCHANGE: {
        TCHAR buf[KL_NAMELENGTH];
        GetKeyboardLayoutName(buf);
        isQWERTY = lstrcmpi(buf, L"00000409") == 0;
        if (!isQWERTY && !hklQWERTY)
            hklQWERTY = LoadKeyboardLayout(L"00000409", KLF_NOTELLSHELL);
      }
    case WM_CHAR:
    case WM_DEADCHAR:
    case WM_SYSCHAR:
    case WM_SYSDEADCHAR:
        return 0;
    case WM_ENTERSIZEMOVE:
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_COMPOSITED);
        return 0;
    case WM_EXITSIZEMOVE:
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) & ~WS_EX_COMPOSITED);
        return 0;
    }
    return __super::HandleMessage(uMsg, wParam, lParam);
}

MainWindow *MainWindow::Create(LPCTSTR szTitle)
{
    MainWindow *self = new MainWindow();
    RECT client = {0, 0, 622, 286};
    AdjustWindowRect(&client, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, FALSE);
    if (self &&
        self->WinCreateWindow(0,
                szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                CW_USEDEFAULT, CW_USEDEFAULT, client.right - client.left,
                client.bottom - client.top, NULL, NULL)) {
        return self;
    }
    delete self;
    return NULL;
}
