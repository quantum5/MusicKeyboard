#pragma once
#ifndef id68C60171_0140_4DE1_B7255EFF557A74F9
#define id68C60171_0140_4DE1_B7255EFF557A74F9

#include <Window.hpp>
#include <PianoControl.hpp>
#include <midifile.h>
#include <mkntapi.h>

#include <mmsystem.h>
#include <commctrl.h>
#include <shellapi.h>

// Some compatibility defines for poor compilers
#ifndef MAPVK_VK_TO_VSC
#define MAPVK_VK_TO_VSC 0
#endif
#ifndef MAPVK_VSC_TO_VK
#define MAPVK_VSC_TO_VK 1
#endif
#ifndef MAPVK_VK_TO_CHAR
#define MAPVK_VK_TO_CHAR 2
#endif
#ifndef GET_KEYSTATE_WPARAM
#define GET_KEYSTATE_WPARAM(wParam) (LOWORD(wParam))
#endif

class MainWindow : public Window {
public:
    virtual LPCTSTR ClassName() { return TEXT("MusicKeyboardMain"); }
    static MainWindow *Create(LPCTSTR szTitle);
    void PlayNote(int note, bool down);
protected:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate();
    LRESULT OnDestroy();
    bool Play(WPARAM wCode, LPARAM lParam, bool down);
    BOOL WinRegisterClass(WNDCLASS *pwc);
    WORD GetQWERTYKeyCode(WORD wKeyCode);
    WORD GetRealKeyCode(WORD wQWERTYCode);
    virtual void PaintContent(PAINTSTRUCT *pps);
    void OnReOpenMIDI();

    virtual HICON GetIcon();

    bool state[128];
    bool useBeep;
    HANDLE hBeep;
    unsigned lastFrequency;
    UNICODE_STRING usBeepDevice;
    T_RtlInitUnicodeString F_RtlInitUnicodeString;
    T_NtCreateFile F_NtCreateFile;

    HWND m_volumeLabel, m_volumeBar;
    HWND m_forceLabel, m_forceBar;
    HWND m_instruLabel, m_instruSelect;
    HWND m_beepCheck;
    HWND m_saveCheck, m_saveLabel, m_saveFile, m_saveBrowse, m_reopen;
    int m_instrument, m_volume, m_force;
    HMIDIOUT m_midi;
    bool isQWERTY;
    HKL hklQWERTY;
    PianoControl *piano;
    LPWSTR m_keychars;
    MIDI_FILE *m_midifile;
    DWORD lastTime;
    bool saving;
private:
    HFONT hFont;
    HBRUSH hBrush;
};

#endif
