#include <MainWindow.hpp>

#include <math.h>
#include <resource.h>
#include <commctrl.h>
#include <commdlg.h>
#include <windowsx.h>

#define LEFT(x, y, cx, cy) x, y, cx, cy
#define RIGHT(x, y, cx, cy) (x - cx), y, cx, cy
#define BOTTOM(x, y, cx, cy) x, (y - cy), cx, cy
#define BOTTOMRIGHT(x, y, cx, cy) (x - cx), (y - cy), cx, cy
#define MIDI_MESSAGE(handle, code, arg1, arg2) \
    midiOutShortMsg(handle, ((arg2 & 0x7F) << 16) |\
                    ((arg1 & 0x7F) << 8) | (code & 0xFF))

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comdlg32.lib")

#include <stdio.h>
#define MessageIntBox(hwnd, i, title, opt) \
    do { \
        char buf[16]; \
        sprintf(buf, "%d", i); \
        MessageBoxA(hwnd, buf, title, opt); \
    } while (0)

static char keymap[256];

struct _keymap_init_class {
    _keymap_init_class() {
        memset(keymap, 0, sizeof keymap);
        keymap[VK_TAB] = 54;
        keymap[VK_OEM_3]  = 55; // `~ key
        keymap['Q']  = 56;
        keymap['A']  = 57;
        keymap['Z']  = 57;
        keymap['W']  = 58;
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
    }
} _keymap_init;

static LPWSTR keychars =
    L"\x21c6\0" // F#3
    L"\x21a5~`\0" // G3
    L"Q\0" // G#3
    L"AZ1\0" // A3
    L"W\0" // A#3
    L"SX2\0" // B3
    L"DC3\0" // C4
    L"R\0" // C#4
    L"FV4\0" // D4
    L"T\0" // D#4
    L"GB5\0" // E4
    L"HN6\0" // F4
    L"U\0" // F#4
    L"JM7\0" // G4
    L"I\0" // G#4
    L"K,8\0" // A4
    L"O\0" // A#4
    L"L.9\0" // B4
    L";/0\0" // C5
    L"[\0" // C#5
    L"\"-\0" // D5
    L"]\0" // D#5
    L"=\x21b5\0" // E5
    L"\x2190\\\0" // F5
;

static WORD frequency[] = {
    8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 19, 21, 22, 23,
    24, 26, 28, 29, 31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62, 65,
    69, 73, 78, 82, 87, 92, 98, 104, 110, 117, 123, 131, 139, 147, 156, 165,
    175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392,
    415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932,
    988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865,
    1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729,
    3951, 4186, 4435, 4699, 4978, 5274, 5588, 5920, 5920, 6645, 7040, 7459,
    7902, 8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544
};

static LPCWSTR majorKeys[] = {
    L"C", L"C#/Db", L"D", L"D#/Eb", L"E", L"F", L"F#/Gb", L"G", L"G#/Ab", L"A", L"A#/Bb", L"B"
};

int dnslen(LPWSTR dns)
{
    LPWSTR i = dns;
    for (; lstrlen(i); i += lstrlen(i) + 1);
    return i - dns;
}

BOOL MainWindow::WinRegisterClass(WNDCLASS *pwc)
{
    pwc->style = CS_HREDRAW | CS_VREDRAW;
    pwc->hbrBackground = (HBRUSH) (COLOR_3DFACE + 1);
    return Window::WinRegisterClass(pwc);
}

LRESULT MainWindow::OnCreate()
{
    NONCLIENTMETRICS ncmMetrics = { sizeof(NONCLIENTMETRICS) };
    RECT client;

    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncmMetrics, 0);
    GetClientRect(m_hwnd, &client);

    hFont = CreateFontIndirect(&ncmMetrics.lfMessageFont);

    // For debugging
    /*AllocConsole();
    freopen("CONOUT$", "w", stdout);*/

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
    m_deviceLabel = CreateWindow(WC_STATIC, L"Device:",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, 0, 0, 0,
            m_hwnd, NULL, GetInstance(), NULL);
    m_adjustLabel = CreateWindow(WC_STATIC, L"Adjustment:",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, 0, 0, 0,
            m_hwnd, NULL, GetInstance(), NULL);
    m_semitoneLabel = CreateWindow(WC_STATIC, L"semitones",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, 0, 0, 0,
            m_hwnd, NULL, GetInstance(), NULL);
    m_octaveLabel = CreateWindow(WC_STATIC, L"octaves",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, 0, 0, 0,
            m_hwnd, NULL, GetInstance(), NULL);
    m_keyLabel = CreateWindow(WC_STATIC, L"Key of:",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, 0, 0, 0,
            m_hwnd, NULL, GetInstance(), NULL);

    m_instruSelect = CreateWindow(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE |
            CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP,
            0, 0, 0, 0,
            m_hwnd, (HMENU) KEYBOARD_INSTRUMENT, GetInstance(), NULL);
    m_forceBar = CreateWindow(TRACKBAR_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0,
            m_hwnd, (HMENU) KEYBOARD_FORCE, GetInstance(), NULL);
    m_volumeBar = CreateWindow(TRACKBAR_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | TBS_NOTICKS | WS_TABSTOP, 0, 0, 0, 0,
            m_hwnd, (HMENU) KEYBOARD_VOLUME, GetInstance(), NULL);
    m_deviceSelect = CreateWindow(WC_COMBOBOX, NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
            0, 0, 0, 0, m_hwnd, (HMENU) KEYBOARD_DEVICE, GetInstance(), NULL);

    m_semitoneSelect = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
            WS_CHILDWINDOW | WS_VISIBLE | ES_NUMBER | ES_LEFT,
            0, 0, 0, 0, m_hwnd, (HMENU) KEYBOARD_SEMITONE, GetInstance(), NULL);
    m_semitoneUpDown = CreateWindow(UPDOWN_CLASS, NULL,
            WS_CHILDWINDOW | WS_VISIBLE | UDS_AUTOBUDDY | UDS_SETBUDDYINT |
                UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_octaveSelect = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
            WS_CHILDWINDOW | WS_VISIBLE | ES_NUMBER | ES_LEFT,
            0, 0, 0, 0, m_hwnd, (HMENU) KEYBOARD_OCTAVE, GetInstance(), NULL);
    m_octaveUpDown = CreateWindow(UPDOWN_CLASS, NULL,
            WS_CHILDWINDOW | WS_VISIBLE | UDS_AUTOBUDDY | UDS_SETBUDDYINT |
                UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_keySelect = CreateWindow(WC_COMBOBOX, NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
            0, 0, 0, 0, m_hwnd, (HMENU) KEYBOARD_KEY, GetInstance(), NULL);

    m_saveCheck = CreateWindow(WC_BUTTON, L"&Save?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | WS_TABSTOP,
            0, 0, 0, 0, m_hwnd, (HMENU) KEYBOARD_SAVE, GetInstance(), NULL);
    m_saveLabel = CreateWindow(WC_STATIC, L"File:",
            WS_CHILD | WS_VISIBLE | WS_DISABLED | SS_CENTERIMAGE, 0, 0, 0, 0,
            m_hwnd, NULL, GetInstance(), NULL);
    m_saveFile = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
            WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_TABSTOP, 0, 0, 0, 0,
            m_hwnd, (HMENU) KEYBOARD_SAVE_FILE, GetInstance(), NULL);
    m_saveBrowse = CreateWindow(WC_BUTTON, L"B&rowse...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED | WS_TABSTOP,
            0, 0, 0, 0, m_hwnd, (HMENU) KEYBOARD_BROWSE, GetInstance(), NULL);
    m_reopen = CreateWindow(WC_BUTTON, L"R&eopen",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED | WS_TABSTOP,
            0, 0, 0, 0, m_hwnd, (HMENU) KEYBOARD_REOPEN, GetInstance(), NULL);
    m_closeFile = CreateWindow(WC_BUTTON, L"&Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED | WS_TABSTOP,
            0, 0, 0, 0, m_hwnd, (HMENU) KEYBOARD_CLOSE_FILE, GetInstance(), NULL);

    m_beepCheck = CreateWindow(WC_BUTTON, L"&Beep?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | WS_TABSTOP,
            0, 0, 0, 0, m_hwnd, (HMENU) KEYBOARD_USE_BEEP, GetInstance(), NULL);

    SendMessage(m_volumeBar, TBM_SETRANGEMIN, FALSE, 0x0000);
    SendMessage(m_volumeBar, TBM_SETRANGEMAX, FALSE, 0xFFFF);
    SendMessage(m_forceBar, TBM_SETRANGE, FALSE, 127 << 16);
    SendMessage(m_volumeBar, TBM_SETPOS, FALSE, 0xFFFF);
    SendMessage(m_forceBar, TBM_SETPOS, FALSE, 64);

    SendMessage(m_semitoneUpDown, UDM_SETRANGE32, (WPARAM) -36, 47);
    SendMessage(m_semitoneUpDown, UDM_SETPOS32, 0, 0);
    SendMessage(m_octaveUpDown, UDM_SETRANGE32, (WPARAM) -3, 3);
    SendMessage(m_octaveUpDown, UDM_SETPOS32, 0, 0);

    m_force = 64;
    m_volume = 0xFFFF;
    m_midifile = NULL;
    m_instrument = 0;
    m_adjust = 0;
    saving = false;
    lastTime = (DWORD) -1;

#define SETFONT(hwnd) SendMessage(hwnd, WM_SETFONT, (WPARAM) hFont, (LPARAM) TRUE)
    SETFONT(m_volumeLabel);
    SETFONT(m_volumeBar);
    SETFONT(m_forceLabel);
    SETFONT(m_forceBar);
    SETFONT(m_instruLabel);
    SETFONT(m_instruSelect);
    SETFONT(m_deviceLabel);
    SETFONT(m_deviceSelect);
    SETFONT(m_adjustLabel);
    SETFONT(m_semitoneSelect);
    SETFONT(m_semitoneLabel);
    SETFONT(m_octaveSelect);
    SETFONT(m_octaveLabel);
    SETFONT(m_keySelect);
    SETFONT(m_keyLabel);
    SETFONT(m_beepCheck);
    SETFONT(m_saveCheck);
    SETFONT(m_saveLabel);
    SETFONT(m_saveFile);
    SETFONT(m_saveBrowse);
    SETFONT(m_reopen);
    SETFONT(m_closeFile);
#undef SETFONT

    SendMessage(m_keySelect, CB_INITSTORAGE, 12, 128);
    for (int i = 0; i < 12; ++i)
        SendMessage(m_keySelect, CB_ADDSTRING, 0, (LPARAM) majorKeys[i]);
    SendMessage(m_keySelect, CB_SETCURSEL, 0, 0);
    {
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
    }
    {
        MIDIOUTCAPS caps;
        deviceCount = midiOutGetNumDevs();
        currentDevice = 0;
        SendMessage(m_deviceSelect, CB_INITSTORAGE, deviceCount, 4096);
        for (int i = 0; i < deviceCount; ++i) {
            midiOutGetDevCaps(i, &caps, sizeof caps);
            SendMessage(m_deviceSelect, CB_ADDSTRING, 0, (LPARAM) caps.szPname);
        }
        SendMessage(m_deviceSelect, CB_SETCURSEL, currentDevice, 0);
    }

    if (midiOutOpen(&m_midi, currentDevice, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
        MessageBox(m_hwnd, L"Failed to open MIDI device!", L"Fatal Error", MB_ICONERROR);

    memset(active, 0, sizeof active);
    this->piano = PianoControl::Create(NULL, m_hwnd, KEYBOARD_IMAGE,
                                       WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                                       0, 0, 0, 0);
    this->piano->SetBackground(GetSysColorBrush(COLOR_3DFACE));

    // Beep Initialization
    F_RtlInitUnicodeString = (T_RtlInitUnicodeString) GetProcAddress(GetModuleHandleA("ntdll"), "RtlInitUnicodeString");
    F_NtCreateFile = (T_NtCreateFile) GetProcAddress(GetModuleHandleA("ntdll"), "NtCreateFile");

    if (!F_RtlInitUnicodeString || !F_NtCreateFile) {
        EnableWindow(m_beepCheck, FALSE);
    } else {
        F_RtlInitUnicodeString(&usBeepDevice, L"\\Device\\Beep");
        hBeep = NULL;
    }
    capsDown = useBeep = adjusting = false;
    m_keychars = NULL;
    PostMessage(m_hwnd, WM_INPUTLANGCHANGE, 0, 0);
    return 0;
}

LRESULT MainWindow::OnDestroy()
{
    midiOutClose(m_midi);
    if (m_midifile)
        midiFileClose(m_midifile);
    return 0;
}

HICON MainWindow::GetIcon()
{
    return LoadIcon(GetInstance(), MAKEINTRESOURCE(RID_ICON));
}

WORD MainWindow::GetQWERTYKeyCode(WORD wKeyCode)
{
    if (isQWERTY)
        return wKeyCode;
    UINT uiScan = MapVirtualKey(wKeyCode, MAPVK_VK_TO_VSC);
    if (!uiScan)
        return wKeyCode;
    UINT uiQWERTY = MapVirtualKeyEx(uiScan, MAPVK_VSC_TO_VK, hklQWERTY);
    if (uiQWERTY)
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
    if (uiKeyCode)
        return (WORD) uiKeyCode;
    else
        return wQWERTYCode;
}

int MainWindow::ModifyNote(int note, bool &half) {
    int state = 0;
    
    if (GetKeyState(VK_CONTROL) < 0)
        state |= 0x001;
    if (GetKeyState(VK_SHIFT) < 0)
        state |= 0x010;
    if (GetKeyState(VK_MENU) < 0)
        state |= 0x100;

    switch (state) {
    case 0x011:
        note -= 24;
        half = false;
        break;
    case 0x001:
        note -= 12;
        half = true;
        break;
    case 0x010:
        note += 12;
        half = true;
        break;
    case 0x100:
        note += 24;
        half = false;
        break;
    case 0x101:
        note += 36;
        half = true;
        break;
    case 0x110:
        note += 48;
        half = false;
        break;
    default:
        half = false;
        break;
    }
    return clamp(note + m_adjust, 0, 127);
}

int MainWindow::GetMIDINote(WPARAM wCode, bool &half, int &base)
{
    base = keymap[wCode];
    return ModifyNote(base, half);
}

bool MainWindow::Play(WPARAM wParam, LPARAM lParam, bool down)
{
    int base, note;
    bool half;
    WORD wCode = GetQWERTYKeyCode((WORD) wParam);
    if (wCode > 255 || !keymap[wCode] || (down && (lParam & 0x40000000)))
        return false;

    note = GetMIDINote(wCode, half, base);
    PlayKeyboardNote(note, half, base, down);
    return true;
}

void MainWindow::PlayKeyboardNote(int note, bool half, int base, bool down) {
    if (active[base] != note)
        PlayNote(active[base], false);
    active[base] = down ? note : 0;
    PlayNote(note, down);
    piano->SetKeyStatus(((note + (half ? 6 : -6) - m_adjust) % 24 + 24) % 24, down);
}

void MainWindow::PlayNote(int note, bool down)
{
    bool save = m_midifile && saving;
    if (save) {
        if (lastTime == (DWORD) -1)
            lastTime = GetTickCount();
        midiTrackAddRest(m_midifile, 1, GetTickCount() - lastTime, TRUE);
        midiTrackAddMsg(m_midifile, 1, down ? msgNoteOn : msgNoteOff, note, m_force);
        lastTime = GetTickCount();
    }

    if (useBeep) {
        BEEP_PARAM param = {frequency[note], (ULONG) -1};

        if (down) {
            OBJECT_ATTRIBUTES ObjectAttributes;
            IO_STATUS_BLOCK IoStatusBlock;
            NTSTATUS status;

            if (hBeep)
                CloseHandle(hBeep);
            lastFrequency = param.Frequency;

            if (param.Frequency < 0x25 || param.Frequency > 0x7FFF)
                return; // Out of range

            InitializeObjectAttributes(&ObjectAttributes, &usBeepDevice, 0, NULL, NULL);
            status = F_NtCreateFile(&hBeep, FILE_READ_DATA | FILE_WRITE_DATA,
                                    &ObjectAttributes, &IoStatusBlock, NULL, 0,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    FILE_OPEN_IF, 0, NULL, 0);
            if (NT_SUCCESS(status)) {
                DeviceIoControl(hBeep, IOCTL_BEEP_SET, &param, sizeof param, NULL, 0, NULL, NULL);
            } else {
                MessageBox(m_hwnd, TEXT("Fail to open the beep device."),
                           TEXT("Fatal Error"), MB_ICONERROR);
            }
        } else {
            // Kill the current beep ONLY if no new beep has hijacked the handle
            if (lastFrequency == param.Frequency && hBeep) {
                CloseHandle(hBeep);
                hBeep = NULL;
            }
        }
    } else
        MIDI_MESSAGE(m_midi, down ? 0x90 : 0x80, note, m_force);
}


void MainWindow::PaintContent(PAINTSTRUCT *pps)
{
    HPEN hOldPen = SelectPen(pps->hdc, GetStockPen(DC_PEN));
    HBRUSH hOldBrush = SelectBrush(pps->hdc, GetSysColorBrush(COLOR_3DFACE));
    RECT client;

    GetClientRect(m_hwnd, &client);
    SetBkColor(pps->hdc, GetSysColor(COLOR_3DFACE));
    SetDCPenColor(pps->hdc, GetSysColor(COLOR_3DHILIGHT));

    RoundRect(pps->hdc, 12, client.bottom - 52, client.right - 12, client.bottom - 12, 5, 5);

    SelectPen(pps->hdc, hOldPen);
    SelectBrush(pps->hdc, hOldBrush);
}

void MainWindow::OnReOpenMIDI()
{
    char cpath[MAX_PATH * 2] = { 0 };
    WCHAR path[MAX_PATH] = { 0 };

    if (!GetDlgItemText(m_hwnd, KEYBOARD_SAVE_FILE, path, MAX_PATH))
        return;

    WideCharToMultiByte(CP_ACP, 0, path, -1, cpath, MAX_PATH * 2, NULL, NULL);

    if (m_midifile)
        midiFileClose(m_midifile);
    lastTime = (DWORD) -1;
    m_midifile = midiFileCreate(cpath, TRUE);
    if (!m_midifile) {
        MessageBox(m_hwnd, L"Can't open the file!\r\n\r\n"
                           L"Check if another program is using it.",
                   L"Error", MB_ICONERROR);
        return;
    }
    midiSongAddTempo(m_midifile, 1, 150);
    midiFileSetTracksDefaultChannel(m_midifile, 1, MIDI_CHANNEL_1);
    midiTrackAddProgramChange(m_midifile, 1, m_instrument);
    midiSongAddSimpleTimeSig(m_midifile, 1, 4, MIDI_NOTE_CROCHET);

    EnableWindow(m_closeFile, TRUE);
    SetFocus(m_hwnd);
}

void MainWindow::OnCloseMIDI()
{
    if (m_midifile)
        midiFileClose(m_midifile);
    m_midifile = NULL;
    EnableWindow(m_closeFile, FALSE);
}

LRESULT CALLBACK MainWindow::LowLevelKeyboardHook(HHOOK hHook, int nCode, WPARAM wParam, LPARAM lParam) {
    KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*) lParam;
    if (p->vkCode == VK_CAPITAL) {
        bool half, down;
        int note = ModifyNote(55, half);
        switch (wParam) {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                down = true;
                if (capsDown)
                    goto finish;
                else
                    capsDown = true;
                break;
            case WM_KEYUP:
            case WM_SYSKEYUP:
                down = false;
                capsDown = false;
                break;
            default:
                goto finish;
        }
        PlayKeyboardNote(note, half, 55, down);
        return 1;
    }
    finish:
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

MainWindow *MainWindow::activeHookWindow = NULL;
HHOOK MainWindow::activeHook = NULL;

LRESULT CALLBACK MainWindow::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && activeHookWindow)
        return activeHookWindow->LowLevelKeyboardHook(activeHook, nCode, wParam, lParam);
    return CallNextHookEx(activeHook, nCode, wParam, lParam);
}

void MainWindow::HookWindow(MainWindow *window, DWORD dwThreadID)
{
    if (activeHook)
        UnhookWindowsHookEx(activeHook);
    activeHook = SetWindowsHookEx(WH_KEYBOARD_LL, MainWindow::LowLevelKeyboardProc, GetModuleHandle(NULL), dwThreadID);
    activeHookWindow = window;
}

void MainWindow::UnhookWindow(MainWindow *window)
{
    if (activeHookWindow == window) {
        UnhookWindowsHookEx(activeHook);
        activeHookWindow = NULL;
    }
}

void MainWindow::OnAdjust()
{
    m_adjust = clamp((int) GetDlgItemInt(m_hwnd, KEYBOARD_SEMITONE, NULL, TRUE), -36, 48);
    //if (!useBeep) MIDI_MESSAGE(m_midi, 0xB0, 122, 0);
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            switch (wParam) {
                case VK_SHIFT:
                case VK_CONTROL:
                case VK_MENU:
                    for (int i = 0; i < 128; ++i) {
                        if (active[i]) {
                            bool half;
                            int note = ModifyNote(i, half);
                            if (note != active[i]) {
                                PlayNote(active[i], false);
                                PlayNote(ModifyNote(i, half), true);
                                active[i] = note;
                            }
                        }
                    }
                    return 0;
            }
    }
    switch (uMsg) {
    case WM_CREATE:
        return OnCreate();
    case WM_DESTROY:
        return OnDestroy();
    case WM_NCDESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE: {
        RECT client;
        HDWP hdwp;
        GetClientRect(m_hwnd, &client);
#define REPOS(hwnd, k) hdwp = DeferWindowPos(hdwp, hwnd, 0, k, SWP_NOACTIVATE|SWP_NOZORDER)
        hdwp = BeginDeferWindowPos(14);
        REPOS(piano->GetHWND(), LEFT(12, 12, client.right - 24, client.bottom - 232));
        REPOS(m_instruLabel,    BOTTOM(12, client.bottom - 187, 70, 25));
        REPOS(m_instruSelect,   BOTTOM(82, client.bottom - 187, client.right - 94, 25));
        REPOS(m_forceLabel,     BOTTOM(12, client.bottom - 157, 70, 25));
        REPOS(m_forceBar,       BOTTOM(82, client.bottom - 157, client.right - 94, 25));
        REPOS(m_volumeLabel,    BOTTOM(12, client.bottom - 127, 70, 25));
        REPOS(m_volumeBar,      BOTTOM(82, client.bottom - 127, client.right - 94, 25));
        REPOS(m_deviceLabel,    BOTTOM(12, client.bottom - 97, 70, 25));
        REPOS(m_deviceSelect,   BOTTOM(82, client.bottom - 97, client.right - 94, 25));
        REPOS(m_adjustLabel,    BOTTOM(12, client.bottom - 67, 70, 25));
        REPOS(m_semitoneSelect, BOTTOM(82, client.bottom - 67, 50, 20));
        REPOS(m_semitoneUpDown, BOTTOM(132, client.bottom - 67, 18, 20));
        REPOS(m_semitoneLabel,  BOTTOM(155, client.bottom - 67, 60, 20));
        REPOS(m_octaveSelect,   BOTTOM(220, client.bottom - 67, 50, 20));
        REPOS(m_octaveUpDown,   BOTTOM(270, client.bottom - 67, 18, 20));
        REPOS(m_octaveLabel,    BOTTOM(293, client.bottom - 67, 50, 20));
        REPOS(m_keyLabel,       BOTTOM(345, client.bottom - 67, 45, 20));
        REPOS(m_keySelect,      BOTTOM(390, client.bottom - 67, 65, 22));
        REPOS(m_saveCheck,      BOTTOM(22, client.bottom - 42, 50, 20));
        REPOS(m_saveLabel,      BOTTOM(27, client.bottom - 19, 30, 20));
        REPOS(m_saveFile,       BOTTOM(62, client.bottom - 17, client.right - 334, 25));
        REPOS(m_saveBrowse,     BOTTOMRIGHT(client.right - 187, client.bottom - 17, 80, 25));
        REPOS(m_reopen,         BOTTOMRIGHT(client.right - 102, client.bottom - 17, 80, 25));
        REPOS(m_closeFile,      BOTTOMRIGHT(client.right - 17, client.bottom - 17, 80, 25));
        REPOS(m_beepCheck,      BOTTOMRIGHT(client.right - 17, client.bottom - 42, 60, 20));
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
                if (m_midifile && saving)
                    midiTrackAddProgramChange(m_midifile, 1, m_instrument);
                return 0;
            case KEYBOARD_DEVICE:
                currentDevice = SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0);
                midiOutClose(m_midi);
                if (midiOutOpen(&m_midi, currentDevice, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
                    return MessageBox(m_hwnd, L"Failed to open MIDI device!", L"Fatal Error", MB_ICONERROR), 0;
                MIDI_MESSAGE(m_midi, 0xC0, m_instrument, 0);
                return 0;
            case KEYBOARD_KEY:
                if (!adjusting) {
                    adjusting = true;
                    SetDlgItemInt(m_hwnd, KEYBOARD_SEMITONE,
                        (int) GetDlgItemInt(m_hwnd, KEYBOARD_OCTAVE, NULL, TRUE) * 12
                        + SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0), TRUE);
                    OnAdjust();
                    adjusting = false;
                }
                break;
            }
            break;
        case BN_CLICKED:
            switch (LOWORD(wParam)) {
                case KEYBOARD_SAVE: {
                    BOOL checked = !IsDlgButtonChecked(m_hwnd, KEYBOARD_SAVE);
                    Button_SetCheck(m_saveCheck, checked);
                    EnableWindow(m_saveLabel, checked);
                    EnableWindow(m_saveFile, checked);
                    EnableWindow(m_saveBrowse, checked);
                    EnableWindow(m_reopen, Edit_GetTextLength(m_saveFile) > 0 ? checked : FALSE);
                    EnableWindow(m_closeFile, m_midifile != NULL);
                    saving = checked == TRUE;
                    break;
                }
                case KEYBOARD_USE_BEEP:
                    useBeep = !IsDlgButtonChecked(m_hwnd, KEYBOARD_USE_BEEP);
                    Button_SetCheck(m_beepCheck, useBeep);
                    break;
                case KEYBOARD_BROWSE: {
                    OPENFILENAME ofn = { sizeof(OPENFILENAME), 0 };
                    WCHAR path[MAX_PATH] = { 0 };

                    ofn.hwndOwner = m_hwnd;
                    ofn.lpstrFilter = L"Standard MIDI Files (*.mid)\0*.mid\0All Files (*.*)\0*.*\0";
                    ofn.lpstrFile = path;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.lpstrTitle = L"Save MIDI Output...";
                    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
                    ofn.lpstrDefExt = L"txt";

                    if (!GetSaveFileName(&ofn))
                        break;

                    SetDlgItemText(m_hwnd, KEYBOARD_SAVE_FILE, path);

                    OnReOpenMIDI();
                    break;
                }
                case KEYBOARD_REOPEN:
                    OnReOpenMIDI();
                    break;
                case KEYBOARD_CLOSE_FILE:
                    OnCloseMIDI();
                    break;
            }
            break;
        case EN_CHANGE:
            switch (LOWORD(wParam)) {
            case KEYBOARD_OCTAVE:
                if (!adjusting) {
                    int o = GetDlgItemInt(m_hwnd, KEYBOARD_OCTAVE, NULL, TRUE);
                    int octave = clamp(o, -3, 3);
                    int semitones = octave * 12 + SendMessage(m_keySelect, CB_GETCURSEL, 0, 0);
                    adjusting = true;
                    if (o != octave)
                        SetDlgItemInt(m_hwnd, KEYBOARD_OCTAVE, octave, TRUE);
                    SetDlgItemInt(m_hwnd, KEYBOARD_SEMITONE, semitones, TRUE);
                    OnAdjust();
                    adjusting = false;
                }
                break;
            case KEYBOARD_SEMITONE:
                if (!adjusting) {
                    int s = GetDlgItemInt(m_hwnd, KEYBOARD_SEMITONE, NULL, TRUE);
                    int semitones = clamp(s, -36, 47);
                    adjusting = true;
                    if (s != semitones)
                        SetDlgItemInt(m_hwnd, KEYBOARD_SEMITONE, semitones, TRUE);
                    SetDlgItemInt(m_hwnd, KEYBOARD_OCTAVE, (WPARAM) floor(semitones / 12.0), TRUE);
                    SendMessage(m_keySelect, CB_SETCURSEL, (semitones % 12 + 12) % 12, 0);
                    OnAdjust();
                    adjusting = false;
                }
                break;
            case KEYBOARD_SAVE_FILE:
                EnableWindow(m_reopen, IsDlgButtonChecked(m_hwnd, KEYBOARD_SAVE) &&
                             Edit_GetTextLength(m_saveFile) > 0);
                EnableWindow(m_closeFile, m_midifile != NULL);
                break;
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
        if (wParam == VK_ESCAPE) {
            HWND hwnd = GetNextDlgTabItem(m_hwnd, GetFocus(), GetKeyState(VK_SHIFT) < 0);
            SetFocus(hwnd);
            return 0;
        }
        if (Play(wParam, lParam, false))
            return 0;
        break;
    case WM_NCACTIVATE:
        if (LOWORD(wParam)) {
            HookWindow(this, 0);
        } else {
            UnhookWindow(this);
        }
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

        int size = dnslen(keychars) + 1, i;
        LPWSTR s;
        if (m_keychars)
            delete [] m_keychars;
        m_keychars = new WCHAR[size];
        for (i = 0; i < size; ++i) {
            WORD scan = VkKeyScanEx(keychars[i], hklQWERTY), vk, ch;
            if (LOBYTE(scan) == -1)
                goto giveup;
            vk = GetRealKeyCode(LOBYTE(scan));
            if (!vk || vk == LOBYTE(scan))
                goto giveup;
            ch = (WCHAR) MapVirtualKey(vk, MAPVK_VK_TO_CHAR);
            if (!ch)
                goto giveup;
            m_keychars[i] = ch;
            continue;

            giveup:
            m_keychars[i] = keychars[i];
        }

        for (s = m_keychars, i = 0; i < 24 && lstrlen(s); s += lstrlen(s) + 1, ++i)
            piano->SetKeyText(i, s);
    }
    case WM_CHAR:
    case WM_DEADCHAR:
    case WM_SYSCHAR:
    case WM_SYSDEADCHAR:
        return 0;
    case WM_ENTERSIZEMOVE:
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_COMPOSITED);
        piano->DisableDraw();
        return 0;
    case WM_EXITSIZEMOVE:
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) & ~WS_EX_COMPOSITED);
        piano->EnableDraw();
        return 0;
    case MMWM_NOTEID: {
        int note = wParam + 54;
        bool half;
        note = ModifyNote(note, half);
        return note;
    }
    case MMWM_TURNNOTE:
        PlayNote((int) wParam, lParam != 0);
        return 0;
    }
    return Window::HandleMessage(uMsg, wParam, lParam);
}

MainWindow *MainWindow::Create(LPCTSTR szTitle)
{
    MainWindow *self = new MainWindow();
    RECT client = {0, 0, 622, 401};
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
