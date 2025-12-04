#ifndef SCREENS_LOGIN_H
#define SCREENS_LOGIN_H

#include <windows.h>
#include <string>

bool runLoginScreen(HANDLE hIn, HANDLE hOut, std::string& username, std::string& password);

#endif // SCREENS_LOGIN_H
