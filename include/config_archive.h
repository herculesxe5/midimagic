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

#ifndef MIDIMAGIC_CONFIG_ARCHIVE_H
#define MIDIMAGIC_CONFIG_ARCHIVE_H

#include "common.h"
#include "system_config.h"
#include "hardware_config.h"
#include "microwire_eeprom.h"
#include "midi_types.h"
#include "output.h"

namespace midimagic {
    class config_archive {
    public:
        config_archive();
        config_archive(const struct system_config system_state);
        ~config_archive();
        config_archive(const config_archive&) = delete;

        enum operation_result {
            SUCCESS = 0,
            NO_ARCHIVE_FOUND,
            VERSION_UNKNOWN,
            SIZE_MISMATCH,
            ARCHIVE_EMPTY,
            CORRUPT_HEADER,
            ILLEGAL_ADDRESS_ON_WRITE,
            ILLEGAL_ADDRESS_ON_READ,
            ILLEGAL_CONFIG_BASE_ADDRESS,
            UNKNOWN_CONFIG_TYPE_ON_READ,
            CONFIG_TOO_BIG
        };

        // copy external system_state into class member
        void readin(const struct system_config system_state);
        // return current saved system state
        const struct system_config spellout() const;
        // deserialise archive into system_state struct
        const operation_result loadon();
        // serialise system_state member into eeprom
        const operation_result writeout();

    private:
        #define RUNNING_VERSION 1
        #define MAGIC 0x4d4d // "MM"

        enum static_header_field : u16 {
            MAGIC0 = 0,
            MAGIC1,
            VERSION,
            SIZE0,
            SIZE1,
            PORT_CONFIG_COUNT,
            PORTGROUP_CONFIG_COUNT,
            FIRST_CONFIG_BASE_ADDR
        };

        enum v1_port_config_field : u16 {
            PORT_NUMBER = 0,
            CLOCK_RATE,
            VELOCITY
        };

        enum v1_portgroup_config_field : u16 {
            DEMUX_TYPE = 0,
            MIDI_CHANNEL,
            CC_NUMBER,
            TRANSPOSE,
            INPUT_TYPE_COUNT,
            OUTPUT_PORTS, // 1 byte bitfield [Port0(MSB),...,Port7(LSB)]
            FIRST_VARIABLE
        };

        enum config_type {
            OUTPUT_PORT_CONFIG = 0,
            PORTGROUP_CONFIG
        };

        // generate header in the eeproom from system_state, return size
        u16 generate_archive_header();
        // read header to return base address of configuration at index, return 0 on failure
        u16 get_address_to(config_type type, u8 index);
        // serialise config struct and write to eeprom, return size
        u16 serialise(struct output_port_config config, u16 base_addr);
        u16 serialise(struct port_group_config config, u16 base_addr);
        // deserialise struct of type from eeprom, write to system_state
        const operation_result deserialise(config_type type, u16 base_addr);

        struct system_config m_system_state;
        microwire_eeprom m_eeprom;
        const u16 k_port_config_size = 3;
        const u16 k_fixed_portgroup_config_size = 6;
        u8 m_running_portgroup_id;
    };
} // namespace midimagic
#endif // MIDIMAGIC_CONFIG_ARCHIVE_H