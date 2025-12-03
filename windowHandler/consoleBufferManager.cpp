#include "consoleBufferManager.h"
#include <windows.h>
#include <iostream>

bool ConsoleBufferManager::AdjustBufferToWindow(HANDLE hOut) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
        std::cout << "GetConsoleScreenBufferInfo() failed! Reason : " << GetLastError() << std::endl;
        return false;
    }

    short winHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    COORD newSize{ csbi.dwSize.X, winHeight };

    if (!SetConsoleScreenBufferSize(hOut, newSize)) {
        std::cout << "SetConsoleScreenBufferSize() failed! Reason : " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

void ConsoleBufferManager::SaveConsoleState(HANDLE hConsole, ConsoleState &state) {
    state.info.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
    GetConsoleScreenBufferInfoEx(hConsole, &state.info);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    state.originalAttributes = csbi.wAttributes;
}

void ConsoleBufferManager::RestoreConsoleState(HANDLE hConsole, const ConsoleState &state) {
    SetConsoleScreenBufferInfoEx(hConsole, const_cast<CONSOLE_SCREEN_BUFFER_INFOEX*>(&state.info));
    SetConsoleTextAttribute(hConsole, state.originalAttributes);
}

bool ConsoleBufferManager::SetExactWindowAndBufferSize(HANDLE hConsole, SHORT width, SHORT height) {
    SMALL_RECT tiny{ 0, 0, 0, 0 };
    if (!SetConsoleWindowInfo(hConsole, TRUE, &tiny)) {
        std::cout << "SetConsoleWindowInfo(1x1) failed! Reason : " << GetLastError() << std::endl;
        return false;
    }

    COORD buf{ width, height };
    if (!SetConsoleScreenBufferSize(hConsole, buf)) {
        std::cout << "SetConsoleScreenBufferSize(target) failed! Reason : " << GetLastError() << std::endl;
        return false;
    }

    SMALL_RECT winRect{ 0, 0, SHORT(width - 1), SHORT(height - 1) };
    if (!SetConsoleWindowInfo(hConsole, TRUE, &winRect)) {
        std::cout << "SetConsoleWindowInfo(target) failed! Reason : " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

bool ConsoleBufferManager::DisableScrolling(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        std::cout << "GetConsoleScreenBufferInfo() failed! Reason : " << GetLastError() << std::endl;
        return false;
    }
    SHORT width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    SHORT height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return SetExactWindowAndBufferSize(hConsole, width, height);
}

bool ConsoleBufferManager::ColorConsole(HANDLE hConsole, COLORREF backgroundRgb, WORD fillAttr) {
    // Save current attribute; we won't override it permanently
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return false;
    WORD originalAttr = csbi.wAttributes;

    // Map palette entries corresponding to the requested background
    CONSOLE_SCREEN_BUFFER_INFOEX csbiex{ sizeof(CONSOLE_SCREEN_BUFFER_INFOEX) };
    if (GetConsoleScreenBufferInfoEx(hConsole, &csbiex)) {
        csbiex.ColorTable[1] = backgroundRgb;
        csbiex.ColorTable[9] = backgroundRgb;
        SetConsoleScreenBufferInfoEx(hConsole, &csbiex);
    }

    DWORD bufferSize = DWORD(csbi.dwSize.X) * DWORD(csbi.dwSize.Y);
    COORD origin{ 0, 0 };
    DWORD written = 0;

    FillConsoleOutputCharacterA(hConsole, ' ', bufferSize, origin, &written);
    FillConsoleOutputAttribute(hConsole, fillAttr, bufferSize, origin, &written);

    // Restore original text attribute
    SetConsoleTextAttribute(hConsole, originalAttr);
    return true;
}

bool ConsoleBufferManager::MaximizeWindowNoScrollbars(HANDLE hConsole) {
    HWND hwnd = GetConsoleWindow();
    if (!hwnd) return false;

    COORD largest = GetLargestConsoleWindowSize(hConsole);
    if (largest.X <= 0 || largest.Y <= 0) return false;
    SMALL_RECT tiny{0,0,0,0};
    SetConsoleWindowInfo(hConsole, TRUE, &tiny);
    COORD buf{ largest.X, largest.Y };
    if (!SetConsoleScreenBufferSize(hConsole, buf)) return false;
    SMALL_RECT maxWin{ 0, 0, SHORT(largest.X - 1), SHORT(largest.Y - 1) };
    if (!SetConsoleWindowInfo(hConsole, TRUE, &maxWin)) return false;
    ShowScrollBar(hwnd, SB_BOTH, FALSE);
    return true;
}

bool ConsoleBufferManager::PrintColor(HANDLE hConsole, COLORREF fg, const char* text) {
    // Use current background color from the console
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        return false;
    WORD attr = csbi.wAttributes;
    // Try to extract the background color as RGB from the VT background if possible
    // Fallback: use black if not available
    COLORREF bg = RGB(0, 0, 0);
    // Optionally, you could store the last used VT background in a static/global if you want perfect match
    return PrintColor(hConsole, fg, bg, text);
}

bool ConsoleBufferManager::PrintColor(HANDLE hConsole, COLORREF fg, COLORREF bg, const char* text) {
    if (!text) return false;
    EnableVirtualTerminal(hConsole);
    BYTE fr = GetRValue(fg), fg_ = GetGValue(fg), fb = GetBValue(fg);
    BYTE br = GetRValue(bg), bg_ = GetGValue(bg), bb = GetBValue(bg);
    char prefix[128];
    int n = snprintf(prefix, sizeof(prefix), "\x1b[38;2;%u;%u;%um\x1b[48;2;%u;%u;%um", (unsigned)fr, (unsigned)fg_, (unsigned)fb, (unsigned)br, (unsigned)bg_, (unsigned)bb);
    if (n < 0) return false;
    DWORD written = 0;
    WriteConsoleA(hConsole, prefix, (DWORD)strlen(prefix), &written, nullptr);
    WriteConsoleA(hConsole, text, (DWORD)strlen(text), &written, nullptr);
    const char* reset = "\x1b[0m";
    WriteConsoleA(hConsole, reset, (DWORD)strlen(reset), &written, nullptr);
    return true;
}

bool ConsoleBufferManager::EnableVirtualTerminal(HANDLE hConsole) {
    DWORD mode = 0;
    if (!GetConsoleMode(hConsole, &mode)) return false;
    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
        DWORD newMode = mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hConsole, newMode);
    }
    return true;
}

bool ConsoleBufferManager::MaximizeAndColor(HANDLE hConsole, COLORREF backgroundRgb, WORD fillAttr) {
    if (!MaximizeWindowNoScrollbars(hConsole)) return false;
    return ColorConsole(hConsole, backgroundRgb, fillAttr);
}
