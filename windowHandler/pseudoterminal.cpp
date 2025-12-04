#include "pseudoterminal.h"
#include <windows.h>
#include <string>

void PseudoTerminal::print(HANDLE hOut, const char* text) {
    DWORD written = 0;
    WriteConsoleA(hOut, text, (DWORD)strlen(text), &written, nullptr);
}

std::string PseudoTerminal::readLine(HANDLE hIn) {
    const DWORD bufSize = 1024;
    char buffer[bufSize];
    DWORD read = 0;
    BOOL ok = ReadConsoleA(hIn, buffer, bufSize - 1, &read, nullptr);
    if (!ok || read == 0) return std::string();
    if (read >= bufSize) read = bufSize - 1;
    buffer[read] = '\0';
    std::string line(buffer);
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
    return line;
}

bool PseudoTerminal::run(HANDLE hIn, HANDLE hOut) {
    // Wait for '!'
    INPUT_RECORD rec;
    DWORD n = 0;
    while (true) {
        if (!ReadConsoleInputA(hIn, &rec, 1, &n)) return false;
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
            const KEY_EVENT_RECORD& kev = rec.Event.KeyEvent;
            if (kev.uChar.AsciiChar == '!') break; // enter pseudo terminal
        }
    }

    // Save original cursor position to restore on ESC
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    GetConsoleScreenBufferInfo(hOut, &csbi);
    COORD originalPos = csbi.dwCursorPosition;

    // Move to bottom line and avoid creating new lines
    COORD bottom{ 0, csbi.srWindow.Bottom };
    SetConsoleCursorPosition(hOut, bottom);
    print(hOut, "\r[pseudo] Type a command (exit to quit, ESC to leave): ");
    print(hOut, "> ");

    // Read line or ESC to leave
    while (true) {
        // Peek for ESC first
        INPUT_RECORD ev;
        DWORD count = 0;
        if (PeekConsoleInputA(hIn, &ev, 1, &count) && count == 1) {
            if (ev.EventType == KEY_EVENT && ev.Event.KeyEvent.bKeyDown && ev.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) {
                // consume this event
                ReadConsoleInputA(hIn, &ev, 1, &count);
                // Restore cursor position to where the user was
                SetConsoleCursorPosition(hOut, originalPos);
                // Clear prompt by overwriting the line
                print(hOut, "\r                                               \r");
                return false; // leave terminal, do not exit program
            }
        }

        // Read a command line
        std::string cmd = readLine(hIn);
        if (cmd.empty()) {
            // nothing, prompt again
            print(hOut, "\r> ");
            continue;
        }
        if (cmd == "exit") {
            // Keep same line, show exit and return
            print(hOut, "\r[pseudo] Exiting program...");
            return true; // signal to exit the app
        }
        // Echo unknown command
        print(hOut, ("\r[pseudo] Unknown command: " + cmd + "\r> ").c_str());
    }
}
