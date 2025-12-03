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
    static bool ColorConsole(HANDLE hConsole, COLORREF backgroundRgb = RGB(0x3e, 0x40, 0x3f), WORD fillAttr = WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY));
    static bool DisableScrolling(HANDLE hConsole);
    static bool SetExactWindowAndBufferSize(HANDLE hConsole, SHORT width, SHORT height);
    static bool MaximizeWindowNoScrollbars(HANDLE hConsole);
    static bool MaximizeAndColor(
        HANDLE hConsole,
        COLORREF backgroundRgb = RGB(0x3e, 0x40, 0x3f),
        WORD fillAttr = WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY)
    );
};

#endif // CONSOLE_BUFFER_MANAGER_H