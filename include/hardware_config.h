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

#ifndef MIDIMAGIC_HARDWARE_CONFIG_H
#define MIDIMAGIC_HARDWARE_CONFIG_H

#include "common.h"
#include "microwire_eeprom.h"

namespace midimagic {
struct hardware {
    struct dac_type {
        const u8 power  = PB12;
        const u8 cs0    = PA0;
        const u8 cs1    = PA1;
        const u8 mosi   = PA7;
        const u8 miso   = PA6;
        const u8 clk    = PA5;
    } const dac;

    struct display_type {
        const i8 sclk = PB10;
        const i8 sdat = PB11;
        const u8 addr = 0x3D;
    } const display;

    struct eeprom_type {
        const u8 mosi = PB0;
        const u8 miso = PB1;
        const u8 clk  = PA2;
        const u8 cs   = PA3;
        const microwire_eeprom::eeprom_size size = microwire_eeprom::eeprom_size::S16Kb;
    } const eeprom;

    /*
    struct midi_type {
        HardwareSerial Serial1;
    } const midi;
    */

    struct ports_type {
        const u8 dpin_port0 = PB9;
        const u8 dpin_port1 = PB8;
        const u8 dpin_port2 = PB7;
        const u8 dpin_port3 = PB6;
        const u8 dpin_port4 = PB5;
        const u8 dpin_port5 = PB4;
        const u8 dpin_port6 = PB3;
        const u8 dpin_port7 = PA15;
    } const ports;

    struct rotary_type {
        const u8 swi = PB15;
        const u8 clk = PB14;
        const u8 dat = PB13;
    } const rotary;


} const hw_setup;

} // namespace midimagic
#endif // MIDIMAGIC_HARDWARE_CONFIG_H