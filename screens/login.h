#ifndef SCREENS_LOGIN_H
#define SCREENS_LOGIN_H

#include <windows.h>
#include <string>

// Renders the login screen and collects username/password.
// Returns true when the user submits; false on failure/abort.
bool runLoginScreen(HANDLE hIn, HANDLE hOut, std::string& username, std::string& password);

#endif // SCREENS_LOGIN_H
