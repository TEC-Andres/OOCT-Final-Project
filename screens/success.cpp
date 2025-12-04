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