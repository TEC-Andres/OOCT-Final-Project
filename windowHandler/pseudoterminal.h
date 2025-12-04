#ifndef PSEUDOTERMINAL_H
#define PSEUDOTERMINAL_H
#include <windows.h>
#include <string>

// Minimal pseudo terminal that can be toggled with '!'
// - When active, reads a command line; 'exit' terminates the app
// - ESC leaves the pseudo terminal and returns control
class PseudoTerminal {
public:
    // Enters pseudo-terminal mode when user presses '!'.
    // Returns true if the command 'exit' was entered (caller should terminate),
    // false otherwise (user pressed ESC to leave or entered other commands).
    bool run(HANDLE hIn, HANDLE hOut);

private:
    std::string readLine(HANDLE hIn);
    void print(HANDLE hOut, const char* text);
};

#endif // PSEUDOTERMINAL_H
