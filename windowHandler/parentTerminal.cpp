#include "parentTerminal.h"
#include <windows.h>

// Keep this helper simple or empty, since we do the heavy lifting in setFullscreen
void ParentTerminal::disableScrolling(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        COORD newSize;
        newSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        newSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        SetConsoleScreenBufferSize(hConsole, newSize);
    }
}

void ParentTerminal::setFullscreen(HANDLE hConsole) {
    HWND hwnd = GetConsoleWindow();
    if (!hwnd) return;

    // 1. Prepare: Expand buffer so window can grow
    COORD tempBufferSize = {300, 9999}; 
    SetConsoleScreenBufferSize(hConsole, tempBufferSize);
    // 2. Action: Maximize
    ShowWindow(hwnd, SW_MAXIMIZE);
    // Without this, we measure the window size before it is fully huge.
    Sleep(60); 

    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style &= ~(WS_VSCROLL | WS_HSCROLL | WS_THICKFRAME);
    SetWindowLongPtr(hwnd, GWL_STYLE, style);
    ShowScrollBar(hwnd, SB_BOTH, FALSE);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    COORD newSize;
    newSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    newSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    SetConsoleScreenBufferSize(hConsole, newSize);
}

void ParentTerminal::saveConsoleState(HANDLE hConsole, ConsoleState &state) {
    state.info.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
    GetConsoleScreenBufferInfoEx(hConsole, &state.info);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    state.originalAttributes = csbi.wAttributes;
}

void ParentTerminal::restoreConsoleState(HANDLE hConsole, const ConsoleState &state) {
    SetConsoleScreenBufferInfoEx(hConsole, const_cast<CONSOLE_SCREEN_BUFFER_INFOEX*>(&state.info));
    SetConsoleTextAttribute(hConsole, state.originalAttributes);
}

void ParentTerminal::colorConsole(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD bufferSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD bufferStart = {0, 0};

    CONSOLE_SCREEN_BUFFER_INFOEX csbiex;
    csbiex.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
    if (GetConsoleScreenBufferInfoEx(hConsole, &csbiex)) {
        COLORREF customBlue = RGB(0x3e, 0x40, 0x3f);
        csbiex.ColorTable[1] = customBlue;
        csbiex.ColorTable[9] = customBlue;
        SetConsoleScreenBufferInfoEx(hConsole, &csbiex);
    }

    WORD cyanAttr = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
    DWORD written;
    
    FillConsoleOutputCharacterA(hConsole, ' ', bufferSize, bufferStart, &written);
    FillConsoleOutputAttribute(hConsole, cyanAttr, bufferSize, bufferStart, &written);
    SetConsoleTextAttribute(hConsole, cyanAttr);
}