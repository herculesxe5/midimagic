/******************************************************************************
 *                                                                            *
 * Copyright 2021 Lukas Jünger and Adrian Krause                              *
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

#ifndef MIDIMAGIC_MENU_INTERFACE_H
#define MIDIMAGIC_MENU_INTERFACE_H

namespace midimagic {
    struct menu_action {
        enum kind {
            UPDATE,
            PORT_ACTIVITY,
            ROT_ACTIVITY,
        };
        enum subkind {
            NO_SUB,
            ROT_LEFT,
            ROT_RIGHT,
            ROT_BUTTON,
            ROT_BUTTON_LONGPRESS,
            PORT_ACTIVE,
            PORT_NACTIVE,
        };
        explicit menu_action(kind k, subkind sk, int d0 = 0, int d1 = 0);
        menu_action() = delete;
        //menu_action(const menu_action&) = delete;
        ~menu_action();
        kind m_kind;
        subkind m_subkind;
        int m_data0;
        int m_data1;
    };

    class menu_interface {
    public:
        menu_interface() {};
        menu_interface(const menu_interface&) = delete;
        virtual ~menu_interface() {};

        virtual void notify(const menu_action &a) = 0;
    };
}

#endif //MIDIMAGIC_MENU_INTERFACE_H