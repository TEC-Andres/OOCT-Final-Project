#include <windows.h>
#include <iostream>
#include "windowHandler/parentTerminal.h"
#include "windowHandler/pseudoterminal.h"
#include "windowHandler/screenTemplate.h"
#include "screens/login.h"
#include "screens/success.h"
#include "__global_vars__.h"

int main()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    ParentTerminal terminal;
    ParentTerminal::ConsoleState state{};
    terminal.saveConsoleState(hOut, state);

    if (!terminal.maximizeAndColor(hOut, HEX(0x2e, 0x30, 0x2f), WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY))) {
        std::cerr << "Failed to maximize and color console." << std::endl;
    }

    // // Use the same background as the one set by colorConsole
    // terminal.printColor(hOut, HEX(0x00, 0xFF, 0x00), bg, "This is a test message in green text on a custom background.\n");
    // terminal.printColor(hOut, HEX(0x00, 0xFF, 0xFF), bg, "This is a test message in cyan text on a custom background.\n");

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    std::string username;
    std::string password;
    bool submitted = runLoginScreen(hIn, hOut, username, password); // Hardcoded screen lol

    // Show success screen only if the password is correct and form submitted
    if (submitted && password == "password") {
        runSuccessScreen(hOut, username);
    }

    // PseudoTerminal test: press '!' to open, ESC to leave; type 'exit' to quit app
    PseudoTerminal pt;
    if (pt.run(hIn, hOut)) {
        terminal.restoreConsoleState(hOut, state);
        return 0;
    }

    terminal.restoreConsoleState(hOut, state);
    return 0;
}

