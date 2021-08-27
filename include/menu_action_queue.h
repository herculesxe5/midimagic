#ifndef MENU_ACTION_QUEUE_H
#define MENU_ACTION_QUEUE_H

#include <queue>
#include <memory>
#include "menu_interface.h"
#include "common.h"

namespace midimagic {
    class menu_action_queue {
    public:
        explicit menu_action_queue(std::shared_ptr<menu_interface> mi);
        menu_action_queue() = delete;
        menu_action_queue(const menu_action_queue&) = delete;
        ~menu_action_queue();

        void add_menu_action(const menu_action& a);
        void exec_next_action();

    private:
        std::queue<menu_action> m_action_queue;
        std::shared_ptr<menu_interface> m_menu;
    };

} // namespace midimagic

#endif //MENU_ACTION_QUEUE_H