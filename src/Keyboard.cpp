#include <MainWindow.hpp>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    InitCommonControls();

    MainWindow *win = MainWindow::Create(L"Music Keyboard");
    if (win) {
        ShowWindow(win->GetHWND(), nCmdShow);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0) {
            if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE) {
                SendMessage(win->GetHWND(), WM_KEYUP, VK_ESCAPE, msg.lParam);
                continue;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}
