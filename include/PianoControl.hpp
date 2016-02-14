#pragma once
#ifndef id7A156843_8DC8_4B5A_83D273814FBE10DE
#define id7A156843_8DC8_4B5A_83D273814FBE10DE

#include <Window.hpp>

#include <mmsystem.h>
#include <commctrl.h>
#include <shellapi.h>

#ifndef WM_TOUCH
    // Because MinGW needs it.
    #define TWF_WANTPALM 0x00000002
    #define WM_TOUCH 0x0240
    #define MICROSOFT_TABLETPENSERVICE_PROPERTY TEXT("MicrosoftTabletPenServiceProperty")
    #define TOUCH_COORD_TO_PIXEL(l) ((l) / 100)
#endif

typedef BOOL (WINAPI *T_GetTouchInputInfo)(HTOUCHINPUT hTouchInput, UINT cInputs, PTOUCHINPUT pInputs, int cbSize);
typedef BOOL (WINAPI *T_CloseTouchInputHandle)(HTOUCHINPUT hTouchInput);
typedef BOOL (WINAPI *T_RegisterTouchWindow)(HWND hWnd, ULONG ulFlags);

#define MPCM_GETKEYSTATUS (WM_USER + 0)
#define MPCM_SETKEYSTATUS (WM_USER + 1)
#define MPCM_GETOCTAVES (WM_USER + 2)
#define MPCM_SETOCTAVES (WM_USER + 3)
#define MPCM_GETKEYTEXT (WM_USER + 4)
#define MPCM_SETKEYTEXT (WM_USER + 5)
#define MPCM_GETBACKGROUND (WM_USER + 6)
#define MPCM_SETBACKGROUND (WM_USER + 7)
#define MMWM_TURNNOTE (WM_APP + 0)
#define MMWM_NOTEID (WM_APP + 1)

#define MAXPOINTS 20

typedef struct {
    bool down;
    POINTL point;
    int lastNote;
    int lastKey;
} PianoTouchPoint;

class PianoControl : public Window {
public:
    virtual LPCTSTR ClassName() { return TEXT("KeyboardControl"); }
    static PianoControl *Create(LPCTSTR szTitle, HWND hwParent, DWORD dwDlgID,
                                DWORD dwStyle = 0,
                                int x = CW_USEDEFAULT, int y = CW_USEDEFAULT,
                                int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT);

    virtual void SetOctaves(int octaves);
    virtual int GetOctaves() { return octaves; }

    virtual void SetKeyStatus(int key, bool down);
    virtual bool GetKeyStatus(int key);

    virtual void SetKeyText(int key, LPCWSTR text);
    virtual LPCWSTR GetKeyText(int key);

    virtual void DisableDraw();
    virtual void EnableDraw();

    virtual void SetBackground(HBRUSH background) { hBackground = background; }
    virtual HBRUSH GetBackground() { return hBackground; }

    HFONT GetFont() { return hFont; }
    void SetFont(HFONT font) { hFont = font; }
protected:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate();
    LRESULT OnDestroy();
    void OnPaint();
    virtual void PaintContent(PAINTSTRUCT *pps);
    BOOL WinRegisterClass(WNDCLASS *pwc);

    void OnTouchPoint(UINT id, int x, int y);
    int GetTouchPointID(DWORD dwID);
    PianoTouchPoint touchPoint[MAXPOINTS];
    DWORD touchPointID[MAXPOINTS];
    bool hasTouch;
    T_GetTouchInputInfo F_GetTouchInputInfo;
    T_CloseTouchInputHandle F_CloseTouchInputHandle;

    virtual int keyIDToInternal(int id, bool &black);
    virtual int internalToKeyID(int id, bool black);
    virtual bool haveBlackToLeft(int id);
    virtual bool haveBlackToRight(int id);
    int haveBlack(int id) {
        return (haveBlackToLeft(id) ? 2 : 0) | (haveBlackToRight(id) ? 1 : 0);
    }
    virtual void UpdateKey(int key, bool black);
    virtual int hitTest(int x, int y, bool &black);

    bool *blackStatus;
    bool *whiteStatus;
    LPCWSTR *blackText;
    LPCWSTR *whiteText;

    int octaves;
    HFONT hFont;
    HWND hwParent;

    HDC hMemDC;
    HBITMAP hMemBitmap;
    HBRUSH hBackground;
    int bmx, bmy;

    bool mouseDown;
    int lastNote, lastKey;
};

#endif
