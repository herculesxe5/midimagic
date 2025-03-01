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
        #define RUNNING_VERSION 2
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

        enum port_config_field : u16 {
            PORT_NUMBER = 0,
            CLOCK_RATE,
            VELOCITY,
            CLOCK_MODE
        };

        enum portgroup_config_field : u16 {
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

        // generate header in the eeprom from system_state, return size
        u16 generate_archive_header();
        // read header to return base address of configuration at index, return 0 on failure
        u16 get_address_to(config_type type, u8 index);
        // serialise config struct and write to eeprom, return size
        u16 serialise(struct output_port_config config, u16 base_addr);
        u16 serialise(struct port_group_config config, u16 base_addr);

        struct system_config m_system_state;
        microwire_eeprom m_eeprom;
        const u16 k_port_config_size = 4;
        const u16 k_fixed_portgroup_config_size = 6;
        u8 m_running_portgroup_id;
    };

    class archive_parser {
    public:
        explicit archive_parser(microwire_eeprom& eeprom);
        archive_parser() = delete;
        archive_parser(const archive_parser&) = delete;
        virtual ~archive_parser();

        virtual const config_archive::operation_result parse() = 0;
        virtual std::unique_ptr<const struct system_config> get_config() const;

    protected:

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

        enum portgroup_config_field : u16 {
            DEMUX_TYPE = 0,
            MIDI_CHANNEL,
            CC_NUMBER,
            TRANSPOSE,
            INPUT_TYPE_COUNT,
            OUTPUT_PORTS, // 1 byte bitfield [Port0(MSB),...,Port7(LSB)]
            FIRST_VARIABLE
        };

        virtual const config_archive::operation_result read_header();

        virtual const struct output_port_config deserialise_port(const u16 base_addr) const = 0;
        virtual const struct port_group_config deserialise_portgroup(const u16 base_addr, const u8 pg_id) const = 0;

        const microwire_eeprom& k_eeprom;
        u16 m_archive_size;
        std::vector<u16> m_port_config_addrs;
        std::vector<u16> m_portgroup_config_addrs;
        struct system_config m_system_config;
    };

    class archive_parser_v1 : public archive_parser {
    public:
        explicit archive_parser_v1(microwire_eeprom& eeprom);
        archive_parser_v1() = delete;
        archive_parser_v1(const archive_parser_v1&) = delete;
        virtual ~archive_parser_v1();

        virtual const config_archive::operation_result parse() override;

    protected:

        enum port_config_field : u16 {
            PORT_NUMBER = 0,
            CLOCK_RATE,
            VELOCITY,
            _FIELD_COUNT_
        };

        virtual const struct output_port_config deserialise_port(const u16 base_addr) const override;
        virtual const struct port_group_config deserialise_portgroup(const u16 base_addr, const u8 pg_id) const override;

        virtual const u8 read_port_number(const u16 base_addr) const;
        virtual const u8 read_port_clock_rate(const u16 base_addr) const;
        virtual const bool read_port_velocity(const u16 base_addr) const;

        virtual const demux_type read_portgroup_demux(const u16 base_addr) const;
        virtual const u8 read_portgroup_chan(const u16 base_addr) const;
        virtual const u8 read_portgroup_cc(const u16 base_addr) const;
        virtual const i8 read_portgroup_transpose(const u16 base_addr) const;
        virtual const std::vector<midi_message::message_type> read_portgroup_msg_types(const u16 base_addr) const;
        virtual const std::vector<u8> read_portgroup_ports(const u16 base_addr) const;
    };

    class archive_parser_v2 : public archive_parser_v1 {
    public:
        explicit archive_parser_v2(microwire_eeprom& eeprom);
        archive_parser_v2() = delete;
        archive_parser_v2(const archive_parser_v2&) = delete;
        virtual ~archive_parser_v2();

    protected:

        enum port_config_field : u16 {
            PORT_NUMBER = 0,
            CLOCK_RATE,
            VELOCITY,
            CLOCK_MODE,
            _FIELD_COUNT_
        };

        virtual const struct output_port_config deserialise_port(const u16 base_addr) const override;

        // new port property added in version 2
        virtual const output_port::clock_mode read_port_clock_mode(const u16 base_addr) const;
    };
} // namespace midimagic
#endif // MIDIMAGIC_CONFIG_ARCHIVE_H