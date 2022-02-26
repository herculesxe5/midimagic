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

#ifndef MIDIMAGIC_ROTARY_H
#define MIDIMAGIC_ROTARY_H

#include "common.h"
#include "menu_action_queue.h"
#include <memory>

namespace midimagic {

    class rotary {
    public:
        explicit rotary(const u8 data_pin, const u8 switch_pin,
                        std::shared_ptr<menu_action_queue> aq);
        rotary() = delete;
        rotary(const rotary&) = delete;
        ~rotary();

        void signal_clk();
        void signal_sw();

    private:
        const u8 m_data_pin;
        const u8 m_switch_pin;
        std::shared_ptr<menu_action_queue> m_action_queue;

        volatile int m_rot_dtstate;
        volatile int m_rot_swstate;
        volatile unsigned long m_current_millis;

        //Rotary encoder ISR timestamps
        volatile unsigned long m_rot_clk_ts;
        volatile unsigned long m_rot_sw_ts;
        //Rotary encoder threshold, ms
        const unsigned int k_rot_int_th = 50;
        //Rotary switch long press duration, ms
        const unsigned int k_rot_lp = 500;
    };
}



#endif //MIDIMAGIC_ROTARY_H
