#include <windows.h>
#include <iostream>
#include "windowHandler/parentTerminal.h"
#include "windowHandler/pseudoterminal.h"
#include "windowHandler/screenTemplate.h"
#include "__global_vars__.h"

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

    // // Use the same background as the one set by colorConsole
    // terminal.printColor(hOut, HEX(0x00, 0xFF, 0x00), bg, "This is a test message in green text on a custom background.\n");
    // terminal.printColor(hOut, HEX(0x00, 0xFF, 0xFF), bg, "This is a test message in cyan text on a custom background.\n");

    // Screen example: username and password
    Screen screen;
    screen.title().setTitle("Secure Large Data Login", Anchor::TOP_CENTER, POS{0,0});
    screen.title().render(hOut);
    screen.text().writeText("Username:", Anchor::CENTER_LEFT, POS{2,-1});
    screen.text().render(hOut);
    screen.textBox().configure(Anchor::CENTER_LEFT, POS{12,-1});
    std::string username = screen.textBox().stringBox(hOut);

    screen.text().writeText("Password:", Anchor::CENTER_LEFT, POS{2,0});
    screen.text().render(hOut);
    screen.textBox().configure(Anchor::CENTER_LEFT, POS{12,0});
    std::string password = screen.textBox().stringBox(hOut);

    // Show captured (demo only; do not print real passwords in production)
    screen.text().writeText(("User: " + username + ", Pass: " + password), Anchor::CENTER_LEFT, POS{2,2});
    screen.text().render(hOut);

    // PseudoTerminal test: press '!' to open, ESC to leave; type 'exit' to quit app
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    PseudoTerminal pt;
    if (pt.run(hIn, hOut)) {
        // Restore state then exit program
        terminal.restoreConsoleState(hOut, state);
        return 0;
    }

    // Restore original console state on exit
    terminal.restoreConsoleState(hOut, state);
    return 0;
}

