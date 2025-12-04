#include "screenTemplate.h"
#include "parentTerminal.h"
#include <windows.h>
#include <string>
#include "../__global_vars__.h"

static COORD toCoord(const POS& p) { return COORD{ p.x, p.y }; }

POS Screen::computePosition(HANDLE hConsole, Anchor anchor, POS offset) {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    SHORT width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    SHORT height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    POS base{};
    switch (anchor) {
        case Anchor::TOP_LEFT:       base = {0, 0}; break;
        case Anchor::TOP_CENTER:     base = {SHORT(width/2), 0}; break;
        case Anchor::TOP_RIGHT:      base = {SHORT(width-1), 0}; break;
        case Anchor::CENTER_LEFT:    base = {0, SHORT(height/2)}; break;
        case Anchor::CENTER:         base = {SHORT(width/2), SHORT(height/2)}; break;
        case Anchor::CENTER_RIGHT:   base = {SHORT(width-1), SHORT(height/2)}; break;
        case Anchor::BOTTOM_LEFT:    base = {0, SHORT(height-1)}; break;
        case Anchor::BOTTOM_CENTER:  base = {SHORT(width/2), SHORT(height-1)}; break;
        case Anchor::BOTTOM_RIGHT:   base = {SHORT(width-1), SHORT(height-1)}; break;
    }
    return POS{ SHORT(base.x + offset.x), SHORT(base.y + offset.y) };
}

// ScreenTitle
void ScreenTitle::setTitle(const std::string& text, Anchor anchor, POS offset, COLORREF hexColor) {
    title_ = text;
    anchor_ = anchor;
    offset_ = offset;
    color_ = hexColor;
}

const std::string& ScreenTitle::getTitle() const { return title_; }

void ScreenTitle::render(HANDLE hConsole) const {
    POS pos = Screen::computePosition(hConsole, anchor_, offset_);
    if (anchor_ == Anchor::TOP_CENTER || anchor_ == Anchor::CENTER || anchor_ == Anchor::BOTTOM_CENTER) {
        int len = static_cast<int>(title_.size());
        int newX = static_cast<int>(pos.x) - (len / 2);
        if (newX < 0) newX = 0;
        pos.x = static_cast<SHORT>(newX);
    }
    SetConsoleCursorPosition(hConsole, toCoord(pos));
    ParentTerminal pt;
    // Use printColor with foreground color, preserving existing background
    pt.printColor(hConsole, color_, bg_title, title_.c_str());
}

// ScreenText
void ScreenText::writeText(const std::string& text, Anchor anchor, POS offset, COLORREF hexColor) {
    text_ = text;
    anchor_ = anchor;
    offset_ = offset;
    color_ = hexColor;
    visible_ = true;
}

void ScreenText::destroyText() { visible_ = false; }

void ScreenText::render(HANDLE hConsole) const {
    if (!visible_) return;
    POS pos = Screen::computePosition(hConsole, anchor_, offset_);
    if (anchor_ == Anchor::TOP_CENTER || anchor_ == Anchor::CENTER || anchor_ == Anchor::BOTTOM_CENTER) {
        int len = static_cast<int>(text_.size());
        int newX = static_cast<int>(pos.x) - (len / 2);
        if (newX < 0) newX = 0;
        pos.x = static_cast<SHORT>(newX);
    }
    SetConsoleCursorPosition(hConsole, toCoord(pos));
    ParentTerminal pt;
    pt.printColor(hConsole, color_, bg, text_.c_str());
}

// ScreenTextBox
void ScreenTextBox::configure(Anchor anchor, POS offset) {
    anchor_ = anchor;
    offset_ = offset;
}

void ScreenTextBox::setLabel(const std::string& label) { label_ = label; }
const std::string& ScreenTextBox::getLabel() const { return label_; }
void ScreenTextBox::setFocused(bool focused) { focused_ = focused; }
bool ScreenTextBox::isFocused() const { return focused_; }

void ScreenTextBox::render(HANDLE hConsole) const {
    ParentTerminal pt;
    POS boxPos = Screen::computePosition(hConsole, anchor_, offset_);
    SetConsoleCursorPosition(hConsole, toCoord(boxPos));
    // Focus highlight: print a background-colored span of spaces, no ASCII box
    COLORREF highlightBg = focused_ ? HEX(0x44,0x44,0x00) : bg; // subtle yellowish bg when focused
    const char* spaces = "                    " ; // width ~20
    pt.printColor(hConsole, HEX(0xFF,0xFF,0xFF), highlightBg, spaces);
    // Position cursor at start of the input area
    SetConsoleCursorPosition(hConsole, toCoord(boxPos));
}

void ScreenTextBox::voidBox(HANDLE hConsole) const {
    POS pos = Screen::computePosition(hConsole, anchor_, offset_);
    SetConsoleCursorPosition(hConsole, toCoord(pos));
    DWORD written = 0;
    const char* blanks = "                    ";
    WriteConsoleA(hConsole, blanks, (DWORD)strlen(blanks), &written, nullptr);
    SetConsoleCursorPosition(hConsole, toCoord(pos));
}

int ScreenTextBox::intBox(HANDLE hConsole) {
    POS pos = Screen::computePosition(hConsole, anchor_, offset_);
    SetConsoleCursorPosition(hConsole, toCoord(pos));
    ParentTerminal pt;
    std::string s = pt.readLinePreserveBackground(hConsole);
    try { return std::stoi(s); } catch (...) { return 0; }
}

std::string ScreenTextBox::stringBox(HANDLE hConsole) {
    POS pos = Screen::computePosition(hConsole, anchor_, offset_);
    SetConsoleCursorPosition(hConsole, toCoord(pos));
    ParentTerminal pt;
    return pt.readLinePreserveBackground(hConsole);
}

int ScreenTextBox::intCell(HANDLE hConsole) { return intBox(hConsole); }
std::string ScreenTextBox::stringCell(HANDLE hConsole) { return stringBox(hConsole); }

// ScreenButton
void ScreenButton::configure(const std::string& text, Anchor anchor, POS offset) {
    text_ = text; anchor_ = anchor; offset_ = offset;
}
void ScreenButton::setFocused(bool focused) { focused_ = focused; }
bool ScreenButton::isFocused() const { return focused_; }
void ScreenButton::render(HANDLE hConsole) const {
    POS pos = Screen::computePosition(hConsole, anchor_, offset_);
    SetConsoleCursorPosition(hConsole, toCoord(pos));
    ParentTerminal pt;
    COLORREF fg = HEX(0xFF,0xFF,0xFF);
    COLORREF bcol = focused_ ? HEX(0x00,0x55,0x00) : bg;
    std::string btn = std::string(" ") + text_ + " ";
    pt.printColor(hConsole, fg, bcol, btn.c_str());
}
void ScreenButton::onActivate(HANDLE hConsole) const {
    // Visual feedback (blink)
    render(hConsole);
}

// Screen composition accessors
ScreenTitle& Screen::title() {
    static ScreenTitle t; return t;
}
ScreenText& Screen::text() {
    static ScreenText tx; return tx;
}
ScreenTextBox& Screen::textBox() {
    static ScreenTextBox tb; return tb;
}
ScreenButton& Screen::button() {
    static ScreenButton b; return b;
}
