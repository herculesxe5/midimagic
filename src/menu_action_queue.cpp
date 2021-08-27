#include "menu_action_queue.h"

namespace midimagic {
    menu_action_queue::menu_action_queue(std::shared_ptr<menu_interface> mi)
        : m_menu(mi) {
        // nothing to do
    }

    menu_action_queue::~menu_action_queue() {
        // nothing to do
    }

    void menu_action_queue::add_menu_action(const menu_action& a) {
        m_action_queue.push(a);
    }

    void menu_action_queue::exec_next_action() {
        if (!m_action_queue.empty()) {
            m_menu->notify(m_action_queue.front());
            m_action_queue.pop();
        }
    }
} // namespace midimagic