#ifndef MOVEMENT_HANDLER_H
#define MOVEMENT_HANDLER_H

#include <windows.h>
#include <functional>
#include <string>
#include <vector>
#include "screenTemplate.h"
#include "parentTerminal.h"

enum class UIItemType { TextBox, Button };

struct UIItem {
    std::string id;
    UIItemType type;
    POS position;
    // Called on every repaint; second arg indicates focused/editing state
    std::function<void(HANDLE hOut, bool focused, bool editing)> render;
    // For text input fields
    std::function<void(HANDLE hIn, HANDLE hOut)> edit;
    // For buttons or submit actions
    std::function<void(HANDLE hOut)> activate;
};

class MovementHandler {
public:
    MovementHandler();

    // Provide items in screen order
    void setItems(const std::vector<UIItem>& items);

    // Run navigation loop; returns index of activated item or -1 if none
    int run(HANDLE hIn, HANDLE hOut, int initialFocus = 0);

    // Force repaint
    void repaint(HANDLE hOut, int focusIndex, bool editing = false);

private:
    std::vector<UIItem> items_;
};

#endif // MOVEMENT_HANDLER_H
