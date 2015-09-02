#include "stdafx.h"
#include "lebenwindow.h"

#include "dwmapi.h"
#include "windowsx.h"
#include "wingdi.h"

LebenWindow::LebenWindow(QWindow *parent)
    : QQuickWindow(parent)
{
    setFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint
          | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setMinimumSize(QSize(640, 480));
}

LebenWindow::~LebenWindow()
{
}

long LebenWindow::hittest(HWND hwnd, MSG* msg) {
    const LONG borderWidth = 8; //in pixels
    RECT winrect;
    GetWindowRect(hwnd, &winrect);
    long x = GET_X_LPARAM(msg->lParam);
    long y = GET_Y_LPARAM(msg->lParam);

    //bottom left corner
    if (x >= winrect.left && x < winrect.left + borderWidth &&
            y < winrect.bottom && y >= winrect.bottom - borderWidth)
    {
        return HTBOTTOMLEFT;
    }
    //bottom right corner
    else if (x < winrect.right && x >= winrect.right - borderWidth &&
            y < winrect.bottom && y >= winrect.bottom - borderWidth)
    {
        return HTBOTTOMRIGHT;
    }
    //top left corner
    else if (x >= winrect.left && x < winrect.left + borderWidth &&
            y >= winrect.top && y < winrect.top + borderWidth)
    {
        return HTTOPLEFT;
    }
    //top right corner
    else if (x < winrect.right && x >= winrect.right - borderWidth &&
            y >= winrect.top && y < winrect.top + borderWidth)
    {
        return HTTOPRIGHT;
    }
    //left border
    else if (x >= winrect.left && x < winrect.left + borderWidth)
    {
        return HTLEFT;
    }
    //right border
    else if (x < winrect.right && x >= winrect.right - borderWidth)
    {
        return HTRIGHT;
    }
    //bottom border
    else if (y < winrect.bottom && y >= winrect.bottom - borderWidth)
    {
        return HTBOTTOM;
    }
    //top border
    else if (y >= winrect.top && y < winrect.top + borderWidth)
    {
        return HTTOP;
    }
    else if (y >= winrect.top && y < winrect.top + 25 + borderWidth &&
             x >= winrect.left && x < winrect.right - 150)
        return HTCAPTION;

    return HTNOWHERE;
}

bool LebenWindow::nativeEvent(const QByteArray &eventType, void *message, long *result) {
    bool dwmPass = false;
    MSG* msg = reinterpret_cast<MSG*>(message);
    dwmPass = QQuickWindow::nativeEvent(eventType, message, result);
    if (dwmPass) return true;
    switch (msg->message) {
    case WM_NCHITTEST:
    {
        *result = hittest((HWND)winId(), msg);
        dwmPass = (*result != 0);
    }
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }
    return dwmPass;
}
