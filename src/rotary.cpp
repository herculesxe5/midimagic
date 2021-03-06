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

#include "rotary.h"

namespace midimagic {

    rotary::rotary(const u8 data_pin, const u8 switch_pin, std::shared_ptr<menu_action_queue> aq)
        : m_data_pin(data_pin)
        , m_switch_pin(switch_pin)
        , m_action_queue(aq) {
        // nothing to do
    }

    rotary::~rotary() {
        // nothing to do
    }

    void rotary::signal_clk() {
        m_rot_dtstate = digitalRead(m_data_pin);
        m_current_millis = millis();
        if (m_current_millis - m_rot_clk_ts > k_rot_int_th) {
            m_rot_clk_ts = m_current_millis;

            if (m_rot_dtstate == HIGH) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_LEFT);
                m_action_queue->add_menu_action(a);
            }
            else if (m_rot_dtstate == LOW) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_RIGHT);
                m_action_queue->add_menu_action(a);
            }
        }
    }

    void rotary::signal_sw() {
        m_rot_swstate = digitalRead(m_switch_pin);
        m_current_millis = millis();
        if (m_rot_swstate == LOW) {
            m_rot_sw_ts = m_current_millis;
        } else {
            if (m_current_millis - m_rot_sw_ts > k_rot_lp) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_BUTTON_LONGPRESS);
                m_action_queue->add_menu_action(a);
            }
            else if (m_current_millis - m_rot_sw_ts > k_rot_int_th) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_BUTTON);
                m_action_queue->add_menu_action(a);
            }
        }
    }
}
