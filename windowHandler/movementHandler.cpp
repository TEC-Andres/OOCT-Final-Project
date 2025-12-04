#include "movementHandler.h"

MovementHandler::MovementHandler() {}

void MovementHandler::setItems(const std::vector<UIItem>& items) {
    items_ = items;
}

void MovementHandler::repaint(HANDLE hOut, int focusIndex, bool editing) {
    for (size_t i = 0; i < items_.size(); ++i) {
        bool focused = (int)i == focusIndex;
        if (items_[i].render) items_[i].render(hOut, focused, editing);
    }
}

int MovementHandler::run(HANDLE hIn, HANDLE hOut, int initialFocus) {
    if (items_.empty()) return -1;

    int focus = (initialFocus >= 0 && initialFocus < (int)items_.size()) ? initialFocus : 0;
    repaint(hOut, focus, false);

    INPUT_RECORD rec{}; DWORD read = 0;
    while (ReadConsoleInput(hIn, &rec, 1, &read)) {
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
            WORD vk = rec.Event.KeyEvent.wVirtualKeyCode;
            switch (vk) {
                case VK_LEFT:
                case VK_UP:
                    focus = (focus + (int)items_.size() - 1) % (int)items_.size();
                    repaint(hOut, focus, false);
                    break;
                case VK_RIGHT:
                case VK_DOWN:
                case VK_TAB:
                    focus = (focus + 1) % (int)items_.size();
                    repaint(hOut, focus, false);
                    break;
                case VK_RETURN: {
                    // Edit if textbox, otherwise activate
                    if (items_[focus].type == UIItemType::TextBox) {
                        repaint(hOut, focus, true);
                        if (items_[focus].edit) items_[focus].edit(hIn, hOut);
                        repaint(hOut, focus, false);
                    } else {
                        if (items_[focus].activate) items_[focus].activate(hOut);
                        return focus; // signal activation
                    }
                } break;
                default:
                    break;
            }
        }
    }
    return -1;
}
