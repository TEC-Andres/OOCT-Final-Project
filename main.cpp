#include <windows.h>
#include <iostream>
#include "windowHandler/parentTerminal.h"

#define HEX(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

int main()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    ParentTerminal terminal;
    ParentTerminal::ConsoleState state{};
    terminal.saveConsoleState(hOut, state);

    // Maximize the window and then apply coloring, keeping scrollbars inaccessible
    if (!terminal.maximizeAndColor(hOut, HEX(0x2e, 0x30, 0x2f), WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY))) {
        std::cerr << "Failed to maximize and color console." << std::endl;
    }

    // Use the same background as the one set by colorConsole
    COLORREF bg = HEX(0x2e, 0x30, 0x2f);
    terminal.printColor(hOut, HEX(0x00, 0xFF, 0x00), bg, "This is a test message in green text on a custom background.\n");
    terminal.printColor(hOut, HEX(0x00, 0xFF, 0xFF), bg, "This is a test message in cyan text on a custom background.\n");

    // Test: prompt for input and preserve background
    terminal.printColor(hOut, HEX(0xFF, 0xFF, 0x00), bg, "Type something (background should not change): ");
    std::string input = terminal.readLine(hOut);
    terminal.printColor(hOut, HEX(0xFF, 0xA5, 0x00), bg, ("You typed: " + input + "\n").c_str());

    // Restore original console state on exit
    terminal.restoreConsoleState(hOut, state);
    return 0;
}

