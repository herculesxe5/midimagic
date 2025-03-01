/******************************************************************************
 *                                                                            *
 * Copyright 2022, 2024 Adrian Krause                                         *
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

#include "microwire_eeprom.h"

namespace midimagic {
    microwire_eeprom::microwire_eeprom(const u8 mosi, const u8 miso, const u8 clk, const u8 cs, const eeprom_size size)
        : k_mosi(mosi)
        , k_miso(miso)
        , k_clk(clk)
        , k_cs(cs)
        , k_eeprom_size(size)
        , m_write_enabled(false) {
        pinMode(k_mosi, OUTPUT);
        pinMode(k_miso, INPUT_PULLDOWN);
        pinMode(k_clk, OUTPUT);
        pinMode(k_cs, OUTPUT);
        digitalWrite(k_mosi, LOW);
        digitalWrite(k_clk, LOW);
        digitalWrite(k_cs, LOW);

        switch (k_eeprom_size) {
            case eeprom_size::S1Kb :
                k_address_lenght = 7;
                break;
            case eeprom_size::S2Kb :
                // 8 address bits plus 1 dummy bit for 2Kb version
            case eeprom_size::S4Kb :
                k_address_lenght = 9;
                break;
            case eeprom_size::S16Kb :
                k_address_lenght = 11;
                break;
            default :
                k_address_lenght = 0;
                break;
        }
    }

    microwire_eeprom::~microwire_eeprom() {
        // nothing to do
    }

    void microwire_eeprom::write(const u16 addr, const u8 data) {
        if ((!m_write_enabled) || (addr >= k_eeprom_size)) {
            return;
        }

        send_preamble();

        // send opcode 01
        digitalWrite(k_mosi, LOW);
        cycle_clock();
        digitalWrite(k_mosi, HIGH);
        cycle_clock();

        // send address
        if (!send_address(addr)) {
            return;
        }

        // send data
        u8 mangle_data = data;
        for (u8 i = 0; i < 8; i++) {
            if (mangle_data & 0x80) {
                digitalWrite(k_mosi, HIGH);
            } else {
                digitalWrite(k_mosi, LOW);
            }
            cycle_clock();
            mangle_data <<= 1;
        }
        digitalWrite(k_mosi, LOW);

        digitalWrite(k_cs, LOW);
        delayMicroseconds(1);
        digitalWrite(k_cs, HIGH);
        wait_for_ready();
        clear_ready();
        return;
    }

    void microwire_eeprom::write_2byte(const u16 addr, const u16 data) {
        write(addr, (u8) ((data >> 8) & 0xff));
        write(addr + 1, (u8) (data & 0xff));
    }

    const u8 microwire_eeprom::read(const u16 addr) const {
        if (addr >= k_eeprom_size) {
            return 0;
        }

        send_preamble();

        // send opcode 10
        digitalWrite(k_mosi, HIGH);
        cycle_clock();
        digitalWrite(k_mosi, LOW);
        cycle_clock();

        // send address
        if (!send_address(addr)) {
            return 0;
        }

        // receive data
        u8 data = 0;
        for (u8 i = 0; i < 8; i++) {
            data <<= 1;
            cycle_clock();
            data |= digitalRead(k_miso);
        }

        digitalWrite(k_cs, LOW);
        return data;
    }

    const u16 microwire_eeprom::read_2byte(const u16 addr) const {
        u16 data = read(addr) << 8;
        return data + read(addr + 1);
    }

    void microwire_eeprom::enable_write() {
        send_preamble();

        // send opcode 00
        digitalWrite(k_mosi, LOW);
        cycle_clock();
        cycle_clock();

        // send enable code 11
        digitalWrite(k_mosi, HIGH);
        cycle_clock();
        cycle_clock();

        // send dummy bits according to address lenght minus 2 code bits
        digitalWrite(k_mosi, LOW);
        for (u8 i = 0; i < (k_address_lenght - 2); i++) {
            cycle_clock();
        }

        digitalWrite(k_cs, LOW);

        m_write_enabled = true;
        return;
    }

    void microwire_eeprom::disable_write() {
        send_preamble();

        // send opcode 00, disable code 00 and dummy bits according to address lenght
        digitalWrite(k_mosi, LOW);
        for (u8 i = 0; i < (k_address_lenght + 2); i++) {
            cycle_clock();
        }

        digitalWrite(k_cs, LOW);
        m_write_enabled = false;
        return;
    }

    const microwire_eeprom::eeprom_size microwire_eeprom::get_size() const {
        return k_eeprom_size;
    }

    void microwire_eeprom::wait_for_ready() const {
        while (!digitalRead(k_miso)) {
            // do nothing
        }
        return;
    }

    void microwire_eeprom::clear_ready() const {
        send_startbit();
        digitalWrite(k_cs, LOW);
        return;
    }

    void microwire_eeprom::send_preamble() const {
        digitalWrite(k_mosi, LOW);
        digitalWrite(k_cs, HIGH);
        send_startbit();
    }

    void microwire_eeprom::send_startbit() const {
        digitalWrite(k_mosi, HIGH);
        cycle_clock();
        digitalWrite(k_mosi, LOW);
    }

    bool microwire_eeprom::send_address(const u16 addr) const {
        if (!k_address_lenght) {
            // something is wrong, reset chip to safe state
            digitalWrite(k_cs, LOW);
            return false;
        }

        u16 mangled_addr = addr << (16 - k_address_lenght);
        for (u8 i = 0; i < k_address_lenght; i++) {
            if (mangled_addr & 0x8000) {
                digitalWrite(k_mosi, HIGH);
            } else {
                digitalWrite(k_mosi, LOW);
            }
            cycle_clock();
            mangled_addr <<= 1;
        }
        return true;
    }

    void microwire_eeprom::cycle_clock() const {
        digitalWrite(k_clk, HIGH);
        // minimum 250 ns settling time needed
        delayMicroseconds(1);
        digitalWrite(k_clk, LOW);
        delayMicroseconds(1);
        return;
    }
} // namespace midimagic
