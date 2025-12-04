#include "parentTerminal.h"
#include <windows.h>
#include <iostream>
#include <string>
#include "pseudoterminal.h"

#define BUFFER_SIZE 4096

bool ParentTerminal::adjustBufferToWindow(HANDLE hOut) {
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

bool ParentTerminal::setExactWindowAndBufferSize(HANDLE hConsole, SHORT width, SHORT height) {
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

bool ParentTerminal::disableScrolling(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        std::cout << "GetConsoleScreenBufferInfo() failed! Reason : " << GetLastError() << std::endl;
        return false;
    }
    SHORT width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    SHORT height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return setExactWindowAndBufferSize(hConsole, width, height);
}

bool ParentTerminal::colorConsole(HANDLE hConsole, COLORREF backgroundRgb, WORD fillAttr) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return false;
    WORD originalAttr = csbi.wAttributes;

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

    SetConsoleTextAttribute(hConsole, originalAttr);
    return true;
}

bool ParentTerminal::maximizeWindowNoScrollbars(HANDLE hConsole) {
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

bool ParentTerminal::printColor(HANDLE hConsole, COLORREF fg, const char* text) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        return false;
    COLORREF bg = RGB(0, 0, 0);
    return printColor(hConsole, fg, bg, text);
}

bool ParentTerminal::printColor(HANDLE hConsole, COLORREF fg, COLORREF bg, const char* text) {
    if (!text) return false;
    enableVirtualTerminal(hConsole);
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

bool ParentTerminal::enableVirtualTerminal(HANDLE hConsole) {
    DWORD mode = 0;
    if (!GetConsoleMode(hConsole, &mode)) return false;
    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
        DWORD newMode = mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hConsole, newMode);
    }
    return true;
}

bool ParentTerminal::maximizeAndColor(HANDLE hConsole, COLORREF backgroundRgb, WORD fillAttr) {
    if (!maximizeWindowNoScrollbars(hConsole)) return false;
    return colorConsole(hConsole, backgroundRgb, fillAttr);
}
bool ParentTerminal::moveCursorToBottom(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return false;
    COORD pos{ 0, csbi.srWindow.Bottom };
    return SetConsoleCursorPosition(hConsole, pos) != 0;
}
std::string ParentTerminal::readLinePreserveBackground(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return std::string();
    }

    WORD oldAttr = csbi.wAttributes;

    // Compute an attribute that keeps the on-screen background but current foreground
    COORD cursor = csbi.dwCursorPosition;
    WORD cellAttr = oldAttr;
    DWORD fetched = 0;
    ReadConsoleOutputAttribute(hConsole, &cellAttr, 1, cursor, &fetched);

    const WORD BG_MASK = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
    const WORD FG_MASK = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
    WORD newAttr = WORD((oldAttr & FG_MASK) | (cellAttr & BG_MASK));
    SetConsoleTextAttribute(hConsole, newAttr);

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(hConsole, oldAttr);
        return std::string();
    }

    const DWORD bufSize = 4096;
    char buffer[bufSize];
    DWORD read = 0;
    BOOL ok = ReadConsoleA(hIn, buffer, bufSize - 1, &read, nullptr);
    if (!ok || read == 0) {
        SetConsoleTextAttribute(hConsole, oldAttr);
        return std::string();
    }

    if (read >= bufSize) read = bufSize - 1;
    buffer[read] = '\0';
    std::string line(buffer);
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();

    SetConsoleTextAttribute(hConsole, oldAttr);
    return line;
}

bool ParentTerminal::drawTopBackgroundLine(HANDLE hConsole, COLORREF bgColor) {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return false;
    SHORT width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    // Move cursor to top-left
    COORD pos{ 0, csbi.srWindow.Top };
    if (!SetConsoleCursorPosition(hConsole, pos)) return false;

    // Enable VT and set only background color
    enableVirtualTerminal(hConsole);
    BYTE br = GetRValue(bgColor), bg = GetGValue(bgColor), bb = GetBValue(bgColor);
    char prefix[64];
    int n = snprintf(prefix, sizeof(prefix), "\x1b[48;2;%u;%u;%um", (unsigned)br, (unsigned)bg, (unsigned)bb);
    if (n < 0) return false;
    DWORD written = 0;
    WriteConsoleA(hConsole, prefix, (DWORD)strlen(prefix), &written, nullptr);

    // Write a full line of spaces with background color
    std::string blanks;
    blanks.assign((size_t)width, ' ');
    WriteConsoleA(hConsole, blanks.c_str(), (DWORD)blanks.size(), &written, nullptr);

    // Reset attributes
    const char* reset = "\x1b[0m";
    WriteConsoleA(hConsole, reset, (DWORD)strlen(reset), &written, nullptr);
    return true;
}