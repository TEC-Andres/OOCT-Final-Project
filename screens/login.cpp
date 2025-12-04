// Login screen implementation
#include <windows.h>
#include <string>
#include <vector>
#include "../windowHandler/screenTemplate.h"
#include "../windowHandler/parentTerminal.h"
#include "../__global_vars__.h"
#include "login.h"

static COORD toCoord(const POS& p) { return COORD{ p.x, p.y }; }

bool runLoginScreen(HANDLE hIn, HANDLE hOut, std::string& username, std::string& password)
{
    Screen screen;
    ParentTerminal ptIO;

    // 1. Setup Static Colors
    // Hover color (Dark Yellow)
    COLORREF colHover = HEX(0x44, 0x44, 0x00);
    // Editing color (Bright Yellow #ffff9a -> R:255 G:255 B:154)
    COLORREF colEdit  = HEX(0xFF, 0xFF, 0x9A); 
    // Text color on top of editing background (Black looks better on bright yellow, usually)
    // But keeping White/Black standard based on your request.
    COLORREF textCol  = HEX(0xFF, 0xFF, 0xFF); 

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

    // ---------------------------------------------------------
    // RENDER LOGIC
    // ---------------------------------------------------------
    // 'focus': 0=user, 1=pass, 2=submit
    // 'isEditing': true if user has pressed Enter and is typing
    auto renderFocusArea = [&](int focus, bool isEditing) {
        
        // Helper to draw labels (Moved inside to ensure layer order)
        auto drawLabel = [&](POS p, const char* text) {
            SetConsoleCursorPosition(hOut, toCoord(p));
            ptIO.printColor(hOut, HEX(0xFF,0xFF,0xFF), bg, text);
        };

        // Helper to draw input boxes
        auto drawInputBox = [&](const POS& p, int myIndex, std::string textContent, bool isPassword) {
            SetConsoleCursorPosition(hOut, toCoord(p));
            
            COLORREF boxBg = bg;
            COLORREF boxFg = HEX(0xFF, 0xFF, 0xFF);

            // Determine Background Color
            if (focus == myIndex) {
                if (isEditing) {
                    boxBg = colEdit;
                    boxFg = HEX(0x00, 0x00, 0x00); // Black text on bright yellow is easier to read
                } else {
                    boxBg = colHover;
                    boxFg = HEX(0xFF, 0xFF, 0xFF);
                }
            }

            // Mask password
            std::string displayStr;
            if (isPassword && !textContent.empty()) {
                displayStr = std::string(textContent.length(), '*');
            } else {
                displayStr = textContent;
            }

            // Padding
            const size_t boxWidth = 20;
            if (displayStr.length() < boxWidth) {
                displayStr.append(boxWidth - displayStr.length(), ' ');
            } else {
                displayStr = displayStr.substr(0, boxWidth);
            }

            ptIO.printColor(hOut, boxFg, boxBg, displayStr.c_str());
        };

        // Draw Labels (Always redraw to prevent ghosting from nearby edits)
        drawLabel(userLblPos, "Username:");
        drawLabel(passLblPos, "Password:");

        // Draw Input Boxes
        drawInputBox(posUser, 0, username, false);
        drawInputBox(posPass, 1, password, true);

        // Draw Submit Button
        SetConsoleCursorPosition(hOut, toCoord(posSubmit));
        COLORREF btnFg = HEX(0xFF,0xFF,0xFF);
        COLORREF btnBg = (focus == 2) ? HEX(0x00,0x55,0x00) : bg;
        ptIO.printColor(hOut, btnFg, btnBg, " Submit ");

        // CURSOR PLACEMENT
        // If editing, place cursor at end of text. If navigating, hide it or place at start.
        if (isEditing) {
            if (focus == 0) {
                COORD c = toCoord(posUser);
                c.X += (SHORT)username.length();
                SetConsoleCursorPosition(hOut, c);
            } else if (focus == 1) {
                COORD c = toCoord(posPass);
                c.X += (SHORT)password.length();
                SetConsoleCursorPosition(hOut, c);
            }
        } else {
            // Park cursor on the Submit button or start of focused box
             if (focus == 0) SetConsoleCursorPosition(hOut, toCoord(posUser));
             else if (focus == 1) SetConsoleCursorPosition(hOut, toCoord(posPass));
             else SetConsoleCursorPosition(hOut, COORD{ SHORT(posSubmit.x + 8), posSubmit.y });
        }
    };

    // ---------------------------------------------------------
    // INPUT CAPTURE LOGIC
    // ---------------------------------------------------------
    auto editField = [&](std::string& buffer, int currentFocusMode) {
        FlushConsoleInputBuffer(hIn);
        
        // Initial render in "Editing" mode (Bright Yellow)
        renderFocusArea(currentFocusMode, true);

        INPUT_RECORD ir;
        DWORD read;

        while (true) {
            ReadConsoleInput(hIn, &ir, 1, &read);
            
            if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
                char ch = ir.Event.KeyEvent.uChar.AsciiChar;
                WORD vk = ir.Event.KeyEvent.wVirtualKeyCode;

                if (vk == VK_RETURN || vk == VK_TAB) {
                    return; // Exit editing mode
                } 
                else if (vk == VK_BACK) {
                    if (!buffer.empty()) {
                        buffer.pop_back();
                        renderFocusArea(currentFocusMode, true); // True = Editing Color
                    }
                }
                else if (ch >= 32 && ch <= 126) { 
                    if (buffer.length() < 20) { 
                        buffer += ch;
                        renderFocusArea(currentFocusMode, true); // True = Editing Color
                    }
                }
            }
        }
    };

    int focus = 0; // 0=user, 1=pass, 2=submit
    username.clear();
    password.clear();
    
    // Initial render: Not editing (False)
    renderFocusArea(focus, false);

    // ---------------------------------------------------------
    // MAIN NAVIGATION LOOP
    // ---------------------------------------------------------
    bool submitted = false;
    INPUT_RECORD rec{}; DWORD read = 0;
    
    while (!submitted && ReadConsoleInput(hIn, &rec, 1, &read)) {
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
            WORD vk = rec.Event.KeyEvent.wVirtualKeyCode;
            switch (vk) {
                case VK_LEFT:
                case VK_UP:
                    focus = (focus + 3 - 1) % 3; 
                    renderFocusArea(focus, false); // False = Hover Color
                    break;
                case VK_RIGHT:
                case VK_DOWN:
                case VK_TAB:
                    focus = (focus + 1) % 3; 
                    renderFocusArea(focus, false); // False = Hover Color
                    break;

                case VK_RETURN: {
                    if (focus == 2) {
                        submitted = true;
                    } 
                    else if (focus == 0) {
                        editField(username, 0);
                        focus = 1; // Move to pass
                        renderFocusArea(focus, false); // Back to Hover
                    } 
                    else if (focus == 1) {
                        editField(password, 1);
                        focus = 2; // Move to submit
                        renderFocusArea(focus, false); // Back to	 Hover
                    }
                } break;
                
                default: break;
            }
        }
    }

    // Do not render debug user/pass here anymore
    return submitted;
}