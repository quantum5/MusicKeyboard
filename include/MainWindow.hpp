#pragma once
#ifndef id68C60171_0140_4DE1_B7255EFF557A74F9
#define id68C60171_0140_4DE1_B7255EFF557A74F9

#include <Window.hpp>
#include <PianoControl.hpp>

#include <mmsystem.h>
#include <commctrl.h>
#include <shellapi.h>

class MainWindow : public Window {
public:
    virtual LPCTSTR ClassName() { return TEXT("MusicKeyboardMain"); }
    static MainWindow *Create(LPCTSTR szTitle);
protected:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate();
    LRESULT OnDestroy();
    bool Play(WPARAM wCode, LPARAM lParam, bool down);
    void OnPaint();
    void PaintContent(PAINTSTRUCT *pps);
    BOOL WinRegisterClass(WNDCLASS *pwc);
    WORD GetQWERTYKeyCode(WORD wKeyCode);
    WORD GetRealKeyCode(WORD wQWERTYCode);

    virtual HICON GetIcon();
    
    HWND m_volumeLabel, m_volumeBar;
    HWND m_forceLabel, m_forceBar;
    HWND m_instruLabel, m_instruSelect;
    int m_instrument, m_volume, m_force;
    HMIDIOUT m_midi;
    bool isQWERTY;
    HKL hklQWERTY;
    PianoControl *piano;
    LPWSTR m_keychars;
private:
    HFONT hFont;
    HBRUSH hBrush;
};

#endif
