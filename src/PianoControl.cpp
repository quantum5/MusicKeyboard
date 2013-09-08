#include <PianoControl.hpp>

#include <windowsx.h>
#include <stdio.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

BOOL PianoControl::WinRegisterClass(WNDCLASS *pwc)
{
    return Window::WinRegisterClass(pwc);
}

LRESULT PianoControl::OnCreate()
{
    NONCLIENTMETRICS ncmMetrics = { sizeof(NONCLIENTMETRICS) };
    RECT client;

    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncmMetrics, 0);
    GetClientRect(m_hwnd, &client);

    hFont = CreateFontIndirect(&ncmMetrics.lfMessageFont);
    hwParent = GetParent(m_hwnd);
    
    hMemDC = NULL;
    hMemBitmap = NULL;
    bmx = bmy = 0;
    
    blackStatus = whiteStatus = NULL;
    blackText = whiteText = NULL;
    
    SetOctaves(2);
    return 0;
}

LRESULT PianoControl::OnDestroy()
{
    if (hMemDC)
        DeleteDC(hMemDC);
    return 0;
}

void PianoControl::SetOctaves(int octaves)
{
    bool *newBlackStatus, *newWhiteStatus;
    LPCWSTR *newBlackText, *newWhiteText;
    
    #define RENEW(type, newname, store) {\
        newname = new type[7 * octaves];\
        if (store) {\
            memcpy(newname, store, min(this->octaves * 7, 7 * octaves) * sizeof(type));\
            delete [] store;\
        } else \
            memset(newname, 0, 7 * octaves * sizeof(type));\
        store = newname;\
    }
    RENEW(bool, newBlackStatus, blackStatus);
    RENEW(bool, newWhiteStatus, whiteStatus);
    RENEW(LPCWSTR, newBlackText, blackText);
    RENEW(LPCWSTR, newWhiteText, whiteText);
    
    this->octaves = octaves;
}

void PianoControl::UpdateKey(int key, bool black)
{
    RECT client;
    int width, height;
    int wwidth, bwidth, bheight, hbwidth;

    GetClientRect(m_hwnd, &client);
    width = client.right - client.left;
    height = client.bottom - client.top;
    wwidth = width / 7 / octaves; // Displaying 14 buttons.
    bwidth = width / 12 / octaves; // smaller
    bheight = height / 2;
    hbwidth = bwidth / 2;
    
    if (black) {
        client.left += (key * wwidth) - hbwidth + 2;
        client.right = client.left + bwidth - 5;
        client.bottom = client.top + bheight;
        InvalidateRect(m_hwnd, &client, FALSE);
    } else {
        client.left += key * wwidth;
        client.right = client.left  + wwidth;
        client.bottom = client.top + height;
        InvalidateRect(m_hwnd, &client, FALSE);
    }
}

void PianoControl::SetKeyStatus(int key, bool down)
{
    bool black;
    int id = keyIDToInternal(key, black);
    
    (black ? blackStatus : whiteStatus)[id] = down;
    UpdateKey(id, black);
}

bool PianoControl::GetKeyStatus(int key)
{
    bool black;
    int id = keyIDToInternal(key, black);
    
    return (black ? blackStatus : whiteStatus)[id];
}

void PianoControl::SetKeyText(int key, LPCWSTR text)
{
    bool black;
    int id = keyIDToInternal(key, black);
    
    (black ? blackText : whiteText)[id] = text;
    UpdateKey(id, black);
}

LPCWSTR PianoControl::GetKeyText(int key)
{
    bool black;
    int id = keyIDToInternal(key, black);
    
    return (black ? blackText : whiteText)[id];
}

int PianoControl::keyIDToInternal(int id, bool &black) {
    switch (id % 12) {
        case 0:
        case 2:
        case 4:
        case 7:
        case 9:
            black = true;
            break;
        default:
            black = false;
    }
    
    int ret = 0;
    switch (id % 12) {
        case 0:
        case 1:
            ret = 0;
            break;
        case 2:
        case 3:
            ret = 1;
            break;
        case 4:
        case 5:
            ret = 2;
            break;
        case 6:
            ret = 3;
            break;
        case 7:
        case 8:
            ret = 4;
            break;
        case 9:
        case 10:
            ret = 5;
            break;
        case 11:
            ret = 6;
            break;
    }
    
    return id / 12 * 7 + ret;
}

bool PianoControl::haveBlackToLeft(int i) {
    switch (i % 7) {
        case 0: // G
        case 1: // A
        case 2: // B
            return true;
        case 3: // C
            return false;
        case 4: // D
        case 5: // E
            return true;
        case 6: // F
            return false;
    }
    return false; // not reached
}

bool PianoControl::haveBlackToRight(int i) {
    switch (i % 7) {
        case 0: // G
        case 1: // A
            return true;
        case 2: // B
            return false;
        case 3: // C
        case 4: // D
            return true;
        case 5: // E
            return false;
        case 6: // F
            return true;
    }
    return false; // not reached
}

void PianoControl::PaintContent(PAINTSTRUCT *pps)
{
    RECT client, rect;
    int width, height;
    int wwidth, bwidth, bheight, hbwidth;
    HDC hdc = pps->hdc;
    HBRUSH hbFace   = GetSysColorBrush(COLOR_3DFACE),
           hbDC     = GetStockBrush(DC_BRUSH),
           hbOriginal;
    HPEN hPenOriginal, hPenDC = GetStockPen(DC_PEN);
    HFONT hFontOriginal = SelectFont(hdc, hFont), hFontNew;
    LPWSTR szBuffer = NULL;
    int bufsize = 0;
    COLORREF textColourOriginal = GetTextColor(hdc),
             backgroundOriginal = SetBkMode(hdc, TRANSPARENT);
    LOGFONT lf;
    GetClientRect(m_hwnd, &client);
    width = client.right - client.left;
    height = client.bottom - client.top;
    wwidth = width / 7 / octaves; // Displaying 14 buttons.
    bwidth = width / 12 / octaves; // smaller
    bheight = height / 2;
    bheight = height / 2;
    hbwidth = bwidth / 2;
    
    hbOriginal = SelectBrush(hdc, hBackground);
    hPenOriginal = SelectPen(hdc, hPenDC);
    
    GetObject(hFont, sizeof(LOGFONT), &lf);
    lf.lfWidth = 0;
    lf.lfHeight = min(bwidth, bheight / 4);
    hFontNew = CreateFontIndirect(&lf);
    SelectFont(hdc, hFontNew);

    #define MoveTo(hdc, x, y) MoveToEx(hdc, x, y, NULL)
    #define CURVE_SIZE 5
    #define CURVE_CIRCLE (2*CURVE_SIZE)
    #define DRAWLINE(x1, y1, x2, y2) (\
        MoveTo(hdc, x1, y1),\
        LineTo(hdc, x2, y2)\
    )
    #define DRAWVERTICAL(length, x, y, color) (\
        SetDCPenColor(hdc, color),\
        DRAWLINE(x, y, x, y + length)\
    )
    #define DRAWHORIZON(length, x, y, color) (\
        SetDCPenColor(hdc, color),\
        DRAWLINE(x, y, x + length, y)\
    )
    #define DRAWBORDER(length, dx, color) (\
        SetDCPenColor(hdc, color),\
        MoveTo(hdc, sx + dx, 0),\
        LineTo(hdc, sx + dx, length),\
        MoveTo(hdc, ex - dx - 1, 0),\
        LineTo(hdc, ex - dx - 1, length)\
    )
    #define DRAWBOX(start, d, height, color) (\
        SetDCPenColor(hdc, color),\
        RoundRect(hdc, sx + d, start - CURVE_SIZE, ex - d, height - d, CURVE_CIRCLE, CURVE_CIRCLE)\
    )
    #define INITIALIZE_PAINT_TEXT(store) \
        int len = lstrlen(store[i]), bufneed = len * 3 + 6; \
        int bufidx = 0; \
        if (bufsize < bufneed) { \
            if (szBuffer) \
                delete [] szBuffer; \
            szBuffer = new WCHAR[bufneed]; \
        } \
        for (LPCWSTR c = store[i]; *c; c++) { \
            szBuffer[bufidx++] = *c; \
            szBuffer[bufidx++] = L'\r'; \
            szBuffer[bufidx++] = L'\n'; \
        } \
        szBuffer[bufidx] = 0;
    #define GETBORDER0(down) (down ? GetSysColor(COLOR_3DLIGHT) : RGB(0, 0, 0))
    #define GETBORDER1(down) (down ? GetSysColor(COLOR_3DSHADOW) : GetSysColor(COLOR_3DDKSHADOW))
    #define GETBORDER2(down) (down ? GetSysColor(COLOR_3DDKSHADOW) : GetSysColor(COLOR_3DSHADOW))
    
    rect.top = height - CURVE_SIZE, rect.bottom = height;
    rect.left = client.left, rect.right = client.right;
    FillRect(hdc, &rect, hBackground);
    
    rect.top = client.top, rect.bottom = client.bottom;
    rect.left = client.right - width % (7 * octaves), rect.right = client.right;
    FillRect(hdc, &rect, hBackground);
    for (int i = 0; i < 7 * octaves; ++i) {
        int sx = i * wwidth, ex = i * wwidth + wwidth - 1;
        bool down = whiteStatus[i];
        
        SelectBrush(hdc, hbDC);
        SetDCBrushColor(hdc, GETBORDER1(down));
        DRAWBOX(bheight, 0, height, GETBORDER0(down));
        SetDCBrushColor(hdc, GETBORDER2(down));
        DRAWBOX(bheight, 1, height, GETBORDER1(down));
        SelectBrush(hdc, hbFace);
        DRAWBOX(bheight, 2, height, GETBORDER2(down));
        
        rect.top = 0, rect.bottom = bheight, rect.left = sx, rect.right = ex;
        FillRect(hdc, &rect, hBackground);
        
        switch (haveBlack(i)) {
            case 0: // none
                DRAWBORDER(bheight, 0, GETBORDER0(down));
                DRAWBORDER(bheight, 1, GETBORDER1(down));
                DRAWBORDER(bheight, 2, GETBORDER2(down));
                break;
            case 1: // right
                DRAWVERTICAL(bheight, sx + 0, 0, GETBORDER0(down));
                DRAWVERTICAL(bheight, sx + 1, 0, GETBORDER1(down));
                DRAWVERTICAL(bheight, sx + 2, 0, GETBORDER2(down));
                DRAWVERTICAL(bheight + 0, ex - hbwidth - 0, 0, GETBORDER0(down));
                DRAWVERTICAL(bheight + 1, ex - hbwidth - 1, 0, GETBORDER1(down));
                DRAWVERTICAL(bheight + 2, ex - hbwidth - 2, 0, GETBORDER2(down));
                DRAWHORIZON(hbwidth - 1, ex - hbwidth - 0, bheight + 0, GETBORDER0(down));
                DRAWHORIZON(hbwidth - 1, ex - hbwidth - 1, bheight + 1, GETBORDER1(down));
                DRAWHORIZON(hbwidth - 1, ex - hbwidth - 2, bheight + 2, GETBORDER2(down));
                rect.top = 0, rect.bottom = bheight, rect.left = sx + 3, rect.right = ex - hbwidth - 2;
                FillRect(hdc, &rect, hbFace);
                break;
            case 2: // left
                DRAWVERTICAL(bheight + 0, sx + hbwidth + 0, 0, GETBORDER0(down));
                DRAWVERTICAL(bheight + 1, sx + hbwidth + 1, 0, GETBORDER1(down));
                DRAWVERTICAL(bheight + 2, sx + hbwidth + 2, 0, GETBORDER2(down));
                DRAWVERTICAL(bheight, ex - 1, 0, GETBORDER0(down));
                DRAWVERTICAL(bheight, ex - 2, 0, GETBORDER1(down));
                DRAWVERTICAL(bheight, ex - 3, 0, GETBORDER2(down));
                DRAWHORIZON(hbwidth + 1, sx + 0, bheight + 0, GETBORDER0(down));
                DRAWHORIZON(hbwidth + 1, sx + 1, bheight + 1, GETBORDER1(down));
                DRAWHORIZON(hbwidth + 1, sx + 2, bheight + 2, GETBORDER2(down));
                rect.top = 0, rect.bottom = bheight, rect.left = sx + hbwidth + 3, rect.right = ex - 3;
                FillRect(hdc, &rect, hbFace);
                break;
            case 3: // both
                DRAWVERTICAL(bheight + 0, sx + hbwidth + 0, 0, GETBORDER0(down));
                DRAWVERTICAL(bheight + 1, sx + hbwidth + 1, 0, GETBORDER1(down));
                DRAWVERTICAL(bheight + 2, sx + hbwidth + 2, 0, GETBORDER2(down));
                DRAWVERTICAL(bheight + 0, ex - hbwidth - 0, 0, GETBORDER0(down));
                DRAWVERTICAL(bheight + 1, ex - hbwidth - 1, 0, GETBORDER1(down));
                DRAWVERTICAL(bheight + 2, ex - hbwidth - 2, 0, GETBORDER2(down));
                DRAWHORIZON(hbwidth + 1, sx + 0, bheight + 0, GETBORDER0(down));
                DRAWHORIZON(hbwidth + 1, sx + 1, bheight + 1, GETBORDER1(down));
                DRAWHORIZON(hbwidth + 1, sx + 2, bheight + 2, GETBORDER2(down));
                DRAWHORIZON(hbwidth - 1, ex - hbwidth - 0, bheight + 0, GETBORDER0(down));
                DRAWHORIZON(hbwidth - 1, ex - hbwidth - 1, bheight + 1, GETBORDER1(down));
                DRAWHORIZON(hbwidth - 1, ex - hbwidth - 2, bheight + 2, GETBORDER2(down));
                rect.top = 0, rect.bottom = bheight, rect.left = sx + hbwidth + 3, rect.right = ex - hbwidth - 2;
                FillRect(hdc, &rect, hbFace);
                break;
        }
        
        if (whiteText[i]) {
            INITIALIZE_PAINT_TEXT(whiteText);
            rect.top = bheight + bheight / 7, rect.bottom = height - bheight / 7;
            rect.left = sx, rect.right = ex;
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawText(hdc, szBuffer, -1, &rect, DT_CENTER);
        }
        
        rect.top = client.top, rect.bottom = client.bottom;
        rect.left = ex, rect.right = ex + 1;
        FillRect(hdc, &rect, hBackground);
    }
    for (int i = 0; i < 7 * octaves; ++i) {
        if (!haveBlackToLeft(i))
            continue;
        int sx = (i * wwidth) - hbwidth + 2, ex = sx + bwidth - 5;
        int kj = bwidth / 4, dc = 128 / kj;
        bool down = blackStatus[i];
        SelectBrush(hdc, hbDC);
        for (int j = 0; j < kj; ++j) {
            int gray = down ? j * dc : (128 - j * dc);
            COLORREF colour = RGB(gray, gray, gray);
            SetDCBrushColor(hdc, colour);
            DRAWBOX(-CURVE_SIZE, j, bheight - 2, colour);
        }
        
        if (blackText[i]) {
            INITIALIZE_PAINT_TEXT(blackText);
            rect.top = bheight / 7, rect.bottom = bheight - bheight / 7;
            rect.left = max(0, sx), rect.right = ex;
            SetTextColor(hdc, RGB(255, 255, 255));
            DrawText(hdc, szBuffer, -1, &rect, DT_CENTER);
        }
    }
    #undef MoveTo
    #undef DRAWLINE
    #undef DRAWVERTICAL
    #undef DRAWHORIZON
    #undef DRAWBORDER
    #undef DRAWBOX
    #undef GETBORDER1
    #undef GETBORDER2
    SelectBrush(hdc, hbOriginal);
    SelectPen(hdc, hPenOriginal);
    DeleteObject(hFontNew);
    SelectFont(hdc, hFontOriginal);
    SetTextColor(hdc, textColourOriginal);
    SetBkMode(hdc, backgroundOriginal);
    if (szBuffer)
        delete szBuffer;
}

void PianoControl::OnPaint()
{
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);
    
    int x = ps.rcPaint.left;
    int y = ps.rcPaint.top;
    int cx = ps.rcPaint.right - ps.rcPaint.left;
    int cy = ps.rcPaint.bottom - ps.rcPaint.top;
    HDC hdc = ps.hdc;
    
    if (!hMemDC)
        hMemDC = CreateCompatibleDC(hdc);
    if (!hMemBitmap)
        hMemBitmap = CreateCompatibleBitmap(hdc, cx + 50, cy + 50);
    if (cx > bmx || cy > bmy) {
        if (hMemBitmap)
            DeleteObject(hMemBitmap);
        hMemBitmap = CreateCompatibleBitmap(hdc, cx + 50, cy + 50);
    }
    if (hMemDC && hMemBitmap) {
        ps.hdc = hMemDC;
        
        HBITMAP hbmPrev = SelectBitmap(hMemDC, hMemBitmap);
        SetWindowOrgEx(hMemDC, x, y, NULL);

        PaintContent(&ps);
        BitBlt(hdc, x, y, cx, cy, hMemDC, x, y, SRCCOPY);

        SelectObject(hMemDC, hbmPrev);
    } else
        PaintContent(&ps);
    EndPaint(m_hwnd, &ps);
}

LRESULT PianoControl::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    case WM_SIZE:
        InvalidateRect(m_hwnd, NULL, TRUE);
        return 0;
    case WM_LBUTTONDOWN:
        SetFocus(hwParent);
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_CHAR:
    case WM_DEADCHAR:
    case WM_SYSCHAR:
    case WM_SYSDEADCHAR:
        return SendMessage(hwParent, uMsg, wParam, lParam);
    case WM_GETFONT:
        return (LRESULT) GetFont();
    case WM_SETFONT:
        SetFont((HFONT) wParam);
        if (LOWORD(lParam))
            InvalidateRect(m_hwnd, NULL, TRUE);
    case MPCM_GETKEYSTATUS:
        return GetKeyStatus(wParam);
    case MPCM_SETKEYSTATUS:
        SetKeyStatus(wParam, lParam != 0);
        return 0;
    case MPCM_GETOCTAVES:
        return GetOctaves();
    case MPCM_SETOCTAVES:
        SetOctaves(wParam);
        return 0;
    case MPCM_GETKEYTEXT:
        return GetOctaves();
    case MPCM_SETKEYTEXT:
        SetOctaves(wParam);
        return 0;
    case MPCM_GETBACKGROUND:
        return (LRESULT) GetBackground();
    case MPCM_SETBACKGROUND:
        SetBackground((HBRUSH) wParam);
        return 0;
    }
    return Window::HandleMessage(uMsg, wParam, lParam);
}

PianoControl *PianoControl::Create(LPCTSTR szTitle, HWND hwParent,
                                   DWORD dwStyle, int x, int y, int cx, int cy)
{
    PianoControl *self = new PianoControl();
    if (self &&
        self->WinCreateWindow(0, szTitle, dwStyle, x, y, cx, cy,
                              hwParent, NULL)) {
        return self;
    }
    delete self;
    return NULL;
}
