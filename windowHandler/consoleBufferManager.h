#ifndef CONSOLE_BUFFER_MANAGER_H
#define CONSOLE_BUFFER_MANAGER_H
#include <windows.h>
#include <iostream>

class ConsoleBufferManager {
public:
    struct ConsoleState {
        CONSOLE_SCREEN_BUFFER_INFOEX info;
        WORD originalAttributes;
    };

    static bool AdjustBufferToWindow(HANDLE hOut);
    static void SaveConsoleState(HANDLE hConsole, ConsoleState &state);
    static void RestoreConsoleState(HANDLE hConsole, const ConsoleState &state);
    // Fill entire console background using palette mapping; restores original text attribute afterwards.
    static bool ColorConsole(HANDLE hConsole, COLORREF backgroundRgb = RGB(0x3e, 0x40, 0x3f), WORD fillAttr = WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY));
    static bool DisableScrolling(HANDLE hConsole);
    static bool SetExactWindowAndBufferSize(HANDLE hConsole, SHORT width, SHORT height);
    static bool MaximizeWindowNoScrollbars(HANDLE hConsole);
    // Print text using an arbitrary RGB foreground without globally overriding colors.
    // Temporarily maps a spare palette slot, prints, then restores palette and attributes.
    static bool PrintColor(HANDLE hConsole, COLORREF rgb, const char* text);
    // Enable ANSI VT processing for truecolor support (Windows 10+)
    static bool EnableVirtualTerminal(HANDLE hConsole);
    static bool MaximizeAndColor(
        HANDLE hConsole,
        COLORREF backgroundRgb = RGB(0x3e, 0x40, 0x3f),
        WORD fillAttr = WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY)
    );
};

#endif // CONSOLE_BUFFER_MANAGER_H