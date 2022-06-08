/******************************************************************************
 *                                                                            *
 * Copyright 2022 Adrian Krause                                               *
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

#ifndef MICROWIRE_EEPROM_H
#define MICROWIRE_EEPROM_H

#include <Arduino.h>
#include "common.h"

namespace midimagic {
    class microwire_eeprom {
    public:
        enum eeprom_size : u16 {
            S1Kb = 128,
            S2Kb = 256,
            S4Kb = 512,
            S16Kb = 2048
        };

        explicit microwire_eeprom(const u8 mosi, const u8 miso, const u8 clk, const u8 cs, const eeprom_size size);
        microwire_eeprom() = delete;
        microwire_eeprom(const microwire_eeprom&) = delete;
        ~microwire_eeprom();

        void write(const u16 addr, const u8 data);
        const u8 read(const u16 addr) const;
        void enable_write();
        void disable_write();
        const eeprom_size get_size() const;

    private:
        const u8 k_mosi;
        const u8 k_miso;
        const u8 k_clk;
        const u8 k_cs;
        const eeprom_size k_eeprom_size;
        u8 k_address_lenght;
        bool m_write_enabled;

        void wait_for_ready() const;
        void clear_ready() const;
        void send_preamble() const;
        void send_startbit() const;
        bool send_address(const u16 addr) const;
        void cycle_clock() const;
    };
} // namespace midimagic

#endif // MICROWIRE_EEPROM_H