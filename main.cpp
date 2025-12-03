#include <windows.h>
#include <iostream>
#include "windowHandler/consoleBufferManager.h"

int main()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Save current console state
    ConsoleBufferManager::ConsoleState state{};
    ConsoleBufferManager::SaveConsoleState(hOut, state);

    // Maximize the window and then apply coloring, keeping scrollbars inaccessible
    if (!ConsoleBufferManager::MaximizeAndColor(hOut, RGB(0x2e, 0x30, 0x2f), WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY))) {
        std::cerr << "Failed to maximize and color console." << std::endl;
    }

    std::cout << "Maximized colored screen ready (no scrollbars). Press Enter to exit..." << std::endl;
    std::cin.get();

    // Restore original console state on exit
    ConsoleBufferManager::RestoreConsoleState(hOut, state);
    return 0;
}
