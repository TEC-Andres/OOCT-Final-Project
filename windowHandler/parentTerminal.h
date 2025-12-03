#ifndef PARENT_TERMINAL_H
#define PARENT_TERMINAL_H
#include <windows.h>

class ParentTerminal {
public:
    struct ConsoleState {
        CONSOLE_SCREEN_BUFFER_INFOEX info;
        WORD originalAttributes;
    };
    
    void saveConsoleState(HANDLE hConsole, ConsoleState &state);
    void restoreConsoleState(HANDLE hConsole, const ConsoleState &state);
    void colorConsole(HANDLE hConsole);
    void disableScrolling(HANDLE hConsole);
    void setFullscreen(HANDLE hConsole);
};

#endif // PARENT_TERMINAL_H