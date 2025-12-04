#ifndef PARENT_TERMINAL_H
#define PARENT_TERMINAL_H
#include <windows.h>
#include <string>

class ParentTerminal {
public:
    struct ConsoleState {
        CONSOLE_SCREEN_BUFFER_INFOEX info;
        WORD originalAttributes;
    };
    
    // State management
    void saveConsoleState(HANDLE hConsole, ConsoleState &state);
    void restoreConsoleState(HANDLE hConsole, const ConsoleState &state);

    // Buffer/window helpers
    bool adjustBufferToWindow(HANDLE hOut);
    bool setExactWindowAndBufferSize(HANDLE hConsole, SHORT width, SHORT height);
    bool disableScrolling(HANDLE hConsole);
    void setFullscreen(HANDLE hConsole);
    bool maximizeWindowNoScrollbars(HANDLE hConsole);

    // Coloring and VT
    bool colorConsole(
        HANDLE hConsole,
        COLORREF backgroundRgb = RGB(0x3e, 0x40, 0x3f),
        WORD fillAttr = WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY)
    );
    bool enableVirtualTerminal(HANDLE hConsole);
    bool printColor(HANDLE hConsole, COLORREF fg, const char* text);
    bool printColor(HANDLE hConsole, COLORREF fg, COLORREF bg, const char* text);
    bool maximizeAndColor(
        HANDLE hConsole,
        COLORREF backgroundRgb = RGB(0x3e, 0x40, 0x3f),
        WORD fillAttr = WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY)
    );

    // Draw a full-width background-colored line at the top row
    bool drawTopBackgroundLine(HANDLE hConsole, COLORREF bgColor);

    // Position cursor at the bottom line (leftmost column) of the visible window
    bool moveCursorToBottom(HANDLE hConsole);

    // Read a line from console without altering the background color.
    // Returns the line (without trailing CR/LF). Uses blocking read.
    std::string readLinePreserveBackground(HANDLE hConsole);
};

#endif // PARENT_TERMINAL_H