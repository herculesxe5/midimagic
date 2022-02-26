/******************************************************************************
 *                                                                            *
 * Copyright 2021 Adrian Krause                                               *
 *                                                                            *
 * This file is part of Midimagic.                                            *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify it    *
 * under the terms of the GNU Lesser General Public License as published by   *
 * the Free Software Foundation, either version 3 of the License,             *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.      *
 *                                                                            *
 ******************************************************************************/

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