#ifndef SCREENS_SUCCESS_H
#define SCREENS_SUCCESS_H

#include <windows.h>
#include <string>

// Renders a minimal success screen with a title and a message.
// Optionally shows the username in the message.
void runSuccessScreen(HANDLE hOut, const std::string& username);

#endif // SCREENS_SUCCESS_H