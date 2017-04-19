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

#define KEYBOARD_IMAGE      0xAA00
#define KEYBOARD_VOLUME     0xAA01
#define KEYBOARD_FORCE      0xAA02
#define KEYBOARD_INSTRUMENT 0xAA03
#define KEYBOARD_DEVICE     0xAA04
#define KEYBOARD_SEMITONE   0xAA05
#define KEYBOARD_OCTAVE     0xAA06
#define KEYBOARD_KEY        0xAA07
#define KEYBOARD_USE_BEEP   0xAAFF
#define KEYBOARD_SAVE       0xAB00
#define KEYBOARD_SAVE_FILE  0xAB01
#define KEYBOARD_BROWSE     0xAB02
#define KEYBOARD_REOPEN     0xAB03
#define KEYBOARD_CLOSE_FILE 0xAB04

template<class T>
T clamp(T v, T a, T b) {
    return v < a ? a : (v > b ? b : v);
}

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
    void PlayKeyboardNote(int note, bool half, int base, bool down);
    BOOL WinRegisterClass(WNDCLASS *pwc);
    WORD GetQWERTYKeyCode(WORD wKeyCode);
    WORD GetRealKeyCode(WORD wQWERTYCode);
    virtual void PaintContent(PAINTSTRUCT *pps);
    void OnReOpenMIDI();
    void OnCloseMIDI();
    void OnAdjust();

    int GetMIDINote(WPARAM wCode, bool &half, int &base);
    int ModifyNote(int note, bool &half);
    virtual HICON GetIcon();

    int active[128];
    bool capsDown;
    bool useBeep;
    HANDLE hBeep;
    unsigned lastFrequency;

    int currentDevice, deviceCount;

    UNICODE_STRING usBeepDevice;
    T_RtlInitUnicodeString F_RtlInitUnicodeString;
    T_NtCreateFile F_NtCreateFile;

    HWND m_volumeLabel, m_volumeBar;
    HWND m_forceLabel, m_forceBar;
    HWND m_instruLabel, m_instruSelect;
    HWND m_deviceLabel, m_deviceSelect;
    HWND m_adjustLabel, m_semitoneSelect, m_semitoneUpDown, m_semitoneLabel;
    HWND m_octaveSelect, m_octaveUpDown, m_octaveLabel;
    HWND m_keyLabel, m_keySelect;
    bool adjusting;
    HWND m_beepCheck;
    HWND m_saveCheck, m_saveLabel, m_saveFile, m_saveBrowse, m_reopen, m_closeFile;
    int m_instrument, m_volume, m_force, m_adjust;
    HMIDIOUT m_midi;
    bool isQWERTY;
    HKL hklQWERTY;
    PianoControl *piano;
    LPWSTR m_keychars;
    MIDI_FILE *m_midifile;
    DWORD lastTime;
    bool saving;

    LRESULT CALLBACK LowLevelKeyboardHook(HHOOK hHook, int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static void HookWindow(MainWindow *window, DWORD dwThreadID);
    static void UnhookWindow(MainWindow *window);
    static MainWindow *activeHookWindow;
    static HHOOK activeHook;
private:
    HFONT hFont;
};

#endif
