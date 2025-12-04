// Login screen implementation
#include <windows.h>
#include <string>
#include <vector>
#include "login.h"
#include "../windowHandler/screenTemplate.h"
#include "../windowHandler/parentTerminal.h"
#include "../__global_vars__.h"
#include "../windowHandler/movementHandler.h"

static COORD toCoord(const POS& p) { return COORD{ p.x, p.y }; }

bool runLoginScreen(HANDLE hIn, HANDLE hOut, std::string& username, std::string& password)
{
    Screen screen;
    ParentTerminal ptIO;

    // 2. Calculate Positions Once
    POS userLblPos = Screen::computePosition(hOut, Anchor::CENTER_LEFT, POS{2,-1});
    POS passLblPos = Screen::computePosition(hOut, Anchor::CENTER_LEFT, POS{2,0});
    POS posUser    = Screen::computePosition(hOut, Anchor::CENTER_LEFT, POS{12,-1});
    POS posPass    = Screen::computePosition(hOut, Anchor::CENTER_LEFT, POS{12,0});
    POS posSubmit  = Screen::computePosition(hOut, Anchor::CENTER_LEFT, POS{2,2});

    // 3. Draw Static Title
    ParentTerminal terminal;
    terminal.drawTopBackgroundLine(hOut, HEX(0x0f, 0x24, 0x24));
    screen.title().setTitle("Secure Large Data Login", Anchor::TOP_CENTER, POS{0,0});
    screen.title().render(hOut);

    // Movement handler with mapped items
    MovementHandler mover;

    // Shared label renderer
    auto drawLabel = [&](POS p, const char* text) {
        SetConsoleCursorPosition(hOut, toCoord(p));
        ptIO.printColor(hOut, HEX(0xFF,0xFF,0xFF), bg, text);
    };

    std::vector<UIItem> items;

    // Username textbox item
    items.push_back(UIItem{
        "username",
        UIItemType::TextBox,
        posUser,
        // render
        [&](HANDLE out, bool focused, bool editing){
            drawLabel(userLblPos, "Username:");
            SetConsoleCursorPosition(out, toCoord(posUser));
            COLORREF boxBg = focused ? (editing ? colEdit : colHover) : textBoxBg;
            COLORREF boxFg = (focused && editing) ? HEX(0x00,0x00,0x00) : textCol;
            std::string display = username;
            const size_t w = 20; if (display.length() < w) display.append(w - display.length(), ' '); else display = display.substr(0, w);
            ptIO.printColor(out, boxFg, boxBg, display.c_str());
            if (editing){ COORD c = toCoord(posUser); c.X += (SHORT)username.length(); SetConsoleCursorPosition(out, c);} else { SetConsoleCursorPosition(out, toCoord(posUser)); }
        },
        // edit
        [&](HANDLE in, HANDLE out){
            FlushConsoleInputBuffer(in);
            INPUT_RECORD ir; DWORD rd;
            while (true){
                ReadConsoleInput(in, &ir, 1, &rd);
                if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown){
                    char ch = ir.Event.KeyEvent.uChar.AsciiChar; WORD vk = ir.Event.KeyEvent.wVirtualKeyCode;
                    if (vk == VK_RETURN || vk == VK_TAB) break;
                    else if (vk == VK_BACK){ if (!username.empty()) username.pop_back(); mover.repaint(out, 0, true); }
                    else if (ch >= 32 && ch <= 126){ if (username.length() < 20){ username += ch; mover.repaint(out, 0, true);} }
                }
            }
        },
        // activate (noop for textbox)
        nullptr
    });

    // Password textbox item
    items.push_back(UIItem{
        "password",
        UIItemType::TextBox,
        posPass,
        [&](HANDLE out, bool focused, bool editing){
            drawLabel(passLblPos, "Password:");
            SetConsoleCursorPosition(out, toCoord(posPass));
            COLORREF boxBg = focused ? (editing ? colEdit : colHover) : textBoxBg;
            COLORREF boxFg = (focused && editing) ? HEX(0x00,0x00,0x00) : textCol;
            std::string masked(password.length(), '*');
            std::string display = masked;
            const size_t w = 20; if (display.length() < w) display.append(w - display.length(), ' '); else display = display.substr(0, w);
            ptIO.printColor(out, boxFg, boxBg, display.c_str());
            if (editing){ COORD c = toCoord(posPass); c.X += (SHORT)password.length(); SetConsoleCursorPosition(out, c);} else { SetConsoleCursorPosition(out, toCoord(posPass)); }
        },
        [&](HANDLE in, HANDLE out){
            FlushConsoleInputBuffer(in);
            INPUT_RECORD ir; DWORD rd;
            while (true){
                ReadConsoleInput(in, &ir, 1, &rd);
                if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown){
                    char ch = ir.Event.KeyEvent.uChar.AsciiChar; WORD vk = ir.Event.KeyEvent.wVirtualKeyCode;
                    if (vk == VK_RETURN || vk == VK_TAB) break;
                    else if (vk == VK_BACK){ if (!password.empty()) password.pop_back(); mover.repaint(out, 1, true); }
                    else if (ch >= 32 && ch <= 126){ if (password.length() < 20){ password += ch; mover.repaint(out, 1, true);} }
                }
            }
        },
        nullptr
    });

    // Submit button item
    items.push_back(UIItem{
        "submit",
        UIItemType::Button,
        posSubmit,
        [&](HANDLE out, bool focused, bool /*editing*/){
            SetConsoleCursorPosition(out, toCoord(posSubmit));
            COLORREF btnFg = HEX(0xFF,0xFF,0xFF);
            COLORREF btnBg = focused ? HEX(0x00,0x55,0x00) : bg;
            ptIO.printColor(out, btnFg, btnBg, " Submit ");
        },
        nullptr,
        // activate: nothing to draw here, activation is handled by run() return
        [&](HANDLE /*out*/){ /* no-op */ }
    });

    mover.setItems(items);
    username.clear(); password.clear();
    int activated = mover.run(hIn, hOut, 0);

    screen.text().writeText(("User: " + username + ", Pass: " + password), Anchor::CENTER_LEFT, POS{2,4});
    screen.text().render(hOut);

    return activated == 2;
}