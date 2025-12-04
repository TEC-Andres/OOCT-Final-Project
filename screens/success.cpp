// Success screen implementation
#include <windows.h>
#include <string>
#include "success.h"
#include "../windowHandler/screenTemplate.h"
#include "../windowHandler/parentTerminal.h"
#include "../__global_vars__.h"

static COORD toCoord(const POS& p) { return COORD{ p.x, p.y }; }

void runSuccessScreen(HANDLE hIn, HANDLE hOut)
{
    // Clear previous screen completely using same background color
    ParentTerminal terminal;
    terminal.colorConsole(hOut, HEX(0x2e, 0x30, 0x2f), WORD(BACKGROUND_BLUE | BACKGROUND_INTENSITY));

    // Draw top background line matching style
    terminal.drawTopBackgroundLine(hOut, HEX(0x0f, 0x24, 0x24));

    // Show title and message
    Screen screen;
    screen.title().setTitle("Login Successful", Anchor::TOP_CENTER, POS{0,0});
    screen.title().render(hOut);

    screen.text().writeText("Welcome! Credentials verified.", Anchor::CENTER, POS{0,0});
    screen.text().render(hOut);

    // Keep the screen until a key press (optional)
    INPUT_RECORD ir; DWORD read;
    HANDLE hStdIn = hIn;
    FlushConsoleInputBuffer(hStdIn);
    while (true) {
        if (ReadConsoleInput(hStdIn, &ir, 1, &read)) {
            if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
                break;
            }
        }
    }
}
#include <windows.h>
#include <string>
#include "success.h"
#include "../windowHandler/screenTemplate.h"
#include "../windowHandler/parentTerminal.h"
#include "../__global_vars__.h"

void runSuccessScreen(HANDLE hOut, const std::string& username)
{
    Screen screen;
    ParentTerminal terminal;

    // Draw a top background line and title
    terminal.drawTopBackgroundLine(hOut, bg_title);
    screen.title().setTitle("Login Successful", Anchor::TOP_CENTER, POS{0,0});
    screen.title().render(hOut);

    // Compose and render a simple message
    std::string message = "Welcome, " + (username.empty() ? std::string("user") : username) + "!";
    screen.text().writeText(message, Anchor::CENTER, POS{0,0});
    screen.text().render(hOut);
}