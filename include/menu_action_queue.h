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