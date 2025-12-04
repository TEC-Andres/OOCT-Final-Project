#ifndef SCREEN_TEMPLATE_H
#define SCREEN_TEMPLATE_H

#include <windows.h>
#include <string>
#include "../__global_vars__.h"

struct POS {
    SHORT x{0};
    SHORT y{0};
};

enum class Anchor {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    CENTER_LEFT,
    CENTER,
    CENTER_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

class ScreenTitle {
public:
    // Optional color parameter; if not provided, a default will be used
    void setTitle(const std::string& text, Anchor anchor, POS offset, COLORREF hexColor = HEX(0xFF,0xFF,0xFF));
    const std::string& getTitle() const;
    // Render title
    void render(HANDLE hConsole) const;

private:
    std::string title_{};
    Anchor anchor_{Anchor::TOP_CENTER};
    POS offset_{};
    COLORREF color_{ HEX(0xFF,0xFF,0xFF) };
};

class ScreenText {
public:
    // Optional color parameter; if not provided, a default will be used
    void writeText(const std::string& text, Anchor anchor, POS offset, COLORREF hexColor = HEX(0xFF,0xFF,0xFF));
    void destroyText();
    void render(HANDLE hConsole) const;

private:
    std::string text_{};
    Anchor anchor_{Anchor::TOP_LEFT};
    POS offset_{};
    bool visible_{false};
    COLORREF color_{ HEX(0xFF,0xFF,0xFF) };
};

class ScreenTextBox {
public:
    // Configure position and anchor for the box
    void configure(Anchor anchor, POS offset);

    // Clear/void the box content on screen
    void voidBox(HANDLE hConsole) const;

    // Typed input helpers (blocking, minimal)
    int intBox(HANDLE hConsole);
    std::string stringBox(HANDLE hConsole);

    // Cell-level (single value) helpers
    int intCell(HANDLE hConsole);
    std::string stringCell(HANDLE hConsole);

private:
    Anchor anchor_{Anchor::CENTER_LEFT};
    POS offset_{};
};

class Screen {
public:
    ScreenTitle& title();
    ScreenText& text();
    ScreenTextBox& textBox();

    // Utility to translate anchor+offset into absolute screen position
    static POS computePosition(HANDLE hConsole, Anchor anchor, POS offset);
};

#endif // SCREEN_TEMPLATE_H
