/******************************************************************************
 *                                                                            *
 * Copyright 2022-2024 Adrian Krause                                          *
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

#include "config_archive.h"

namespace midimagic {
    config_archive::config_archive()
        : m_eeprom(hw_setup.eeprom.mosi, hw_setup.eeprom.miso, hw_setup.eeprom.clk, hw_setup.eeprom.cs, hw_setup.eeprom.size)
        , m_running_portgroup_id(0) {
        // nothing to do
    }

    config_archive::config_archive(const struct system_config system_state)
        : m_eeprom(hw_setup.eeprom.mosi, hw_setup.eeprom.miso, hw_setup.eeprom.clk, hw_setup.eeprom.cs, hw_setup.eeprom.size)
        , m_running_portgroup_id(0) {
        readin(system_state);
    }

    config_archive::~config_archive() {
        // nothing to do
    }

    void config_archive::readin(const struct system_config system_state) {
        m_system_state = system_state;
    }

    const struct system_config config_archive::spellout() const {
        return m_system_state;
    }

    const config_archive::operation_result config_archive::loadon() {
        // check for magic
        if (m_eeprom.read_2byte(static_header_field::MAGIC0) != MAGIC) {
            // no joy, abort
            return operation_result::NO_ARCHIVE_FOUND;
        }

        // check version and parse archive
        std::unique_ptr<archive_parser> parser;
        switch (m_eeprom.read(static_header_field::VERSION)) {
            case 1 :
                parser = std::make_unique<archive_parser_v1>(m_eeprom);
                break;
            case 2 :
                parser = std::make_unique<archive_parser_v2>(m_eeprom);
                break;
            default :
                return operation_result::VERSION_UNKNOWN;
                break;
        }

        auto result = parser->parse();

        if (result == operation_result::SUCCESS) {
            m_system_state = std::move(*(parser->get_config().release()));
        }
        return result;
    }

    const config_archive::operation_result config_archive::writeout() {
        m_eeprom.enable_write();

        u16 header_size = generate_archive_header();

        // check if config fits
        if (m_eeprom.read_2byte(static_header_field::SIZE0) > hw_setup.eeprom.size) {
            m_eeprom.disable_write();
            return operation_result::CONFIG_TOO_BIG;
        }

        u16 return_config_size;

        for (u8 index = 0; index < m_system_state.system_ports.size(); index++) {
            u16 next_address = get_address_to(config_type::OUTPUT_PORT_CONFIG, index);
            if (next_address) {
                return_config_size = serialise(m_system_state.system_ports.at(index), next_address);
            } else {
                m_eeprom.disable_write();
                return operation_result::ILLEGAL_ADDRESS_ON_WRITE;
            }
            if (!return_config_size) {
                m_eeprom.disable_write();
                return operation_result::ILLEGAL_CONFIG_BASE_ADDRESS;
            }
        }

        for (u8 index = 0; index < m_system_state.system_port_groups.size(); index++) {
            u16 next_address = get_address_to(config_type::PORTGROUP_CONFIG, index);
            if (next_address) {
                return_config_size = serialise(m_system_state.system_port_groups.at(index), next_address);
            } else {
                m_eeprom.disable_write();
                return operation_result::ILLEGAL_ADDRESS_ON_WRITE;
            }
            if (!return_config_size) {
                m_eeprom.disable_write();
                return operation_result::ILLEGAL_CONFIG_BASE_ADDRESS;
            }
        }
        m_eeprom.disable_write();
        return operation_result::SUCCESS;
    }

    u16 config_archive::generate_archive_header() {
        // write magic
        m_eeprom.write_2byte(static_header_field::MAGIC0, MAGIC);
        // write version
        m_eeprom.write(static_header_field::VERSION, RUNNING_VERSION);

        // write config counts
        u8 port_config_count = m_system_state.system_ports.size();
        u8 portgroup_config_count = m_system_state.system_port_groups.size();
        m_eeprom.write(static_header_field::PORT_CONFIG_COUNT, port_config_count);
        m_eeprom.write(static_header_field::PORTGROUP_CONFIG_COUNT, portgroup_config_count);

        // calculate addresses
        u16 header_size = static_header_field::FIRST_CONFIG_BASE_ADDR + (2 * port_config_count) + (2 * portgroup_config_count);
        u16 running_config_base_addr = header_size;
        u16 running_header_field_addr = static_header_field::FIRST_CONFIG_BASE_ADDR;

        // write port config addresses
        for (u8 i = 0; i < port_config_count; i++) {
            m_eeprom.write_2byte(running_header_field_addr, running_config_base_addr);
            running_config_base_addr += k_port_config_size;
            running_header_field_addr += 2;
        }

        // write portgroup config addresses
        for (auto &pg_config: m_system_state.system_port_groups) {
            m_eeprom.write_2byte(running_header_field_addr, running_config_base_addr);
            running_config_base_addr += k_fixed_portgroup_config_size + (pg_config.input_types.size());
            running_header_field_addr += 2;
        }

        // write total size
        // running_config_base_addr points to the next free cell after the last config which is also the size of the whole archive
        m_eeprom.write_2byte(static_header_field::SIZE0, running_config_base_addr);

        return header_size;
    }

    u16 config_archive::get_address_to(config_type type, u8 index) {
        u8 port_config_count = m_eeprom.read(static_header_field::PORT_CONFIG_COUNT);
        u8 portgroup_config_count = m_eeprom.read(static_header_field::PORTGROUP_CONFIG_COUNT);
        u16 out_addr = 0;
        u16 header_field_addr = 0;
        switch (type) {
            case config_type::OUTPUT_PORT_CONFIG :
                if (index >= port_config_count) {
                    return out_addr;
                }
                header_field_addr = static_header_field::FIRST_CONFIG_BASE_ADDR + (index * 2);
                break;
            case config_type::PORTGROUP_CONFIG :
                if (index >= portgroup_config_count) {
                    return out_addr;
                }
                header_field_addr = static_header_field::FIRST_CONFIG_BASE_ADDR + (port_config_count * 2) + (index * 2);
                break;
            default :
                return out_addr;
                break;
        }
        out_addr = m_eeprom.read_2byte(header_field_addr);
        return out_addr;
    }

    u16 config_archive::serialise(struct output_port_config config, u16 base_addr) {
        if (base_addr < static_header_field::FIRST_CONFIG_BASE_ADDR + 2) {
            // illegal address, no configs exist in this case, nope out...
            return 0;
        }
        m_eeprom.write(base_addr + port_config_field::PORT_NUMBER, config.port_number);
        m_eeprom.write(base_addr + port_config_field::CLOCK_RATE, config.clock_rate);
        m_eeprom.write(base_addr + port_config_field::VELOCITY, config.velocity_output);
        m_eeprom.write(base_addr + port_config_field::CLOCK_MODE, config.clock_mode);
        return k_port_config_size;
    }

    u16 config_archive::serialise(struct port_group_config config, u16 base_addr) {
        if (base_addr < static_header_field::FIRST_CONFIG_BASE_ADDR + 2) {
            // illegal address, no configs exist in this case, nope out...
            return 0;
        }
        u16 configuration_size = k_fixed_portgroup_config_size;

        m_eeprom.write(base_addr + portgroup_config_field::DEMUX_TYPE, config.demux);
        m_eeprom.write(base_addr + portgroup_config_field::MIDI_CHANNEL, config.midi_channel);
        m_eeprom.write(base_addr + portgroup_config_field::CC_NUMBER, config.cont_controller_number);
        if (config.transpose_offset < 0) {
            m_eeprom.write(base_addr + portgroup_config_field::TRANSPOSE, (u8) (0x80 | (config.transpose_offset & 0x7f)));
        } else {
            m_eeprom.write(base_addr + portgroup_config_field::TRANSPOSE, config.transpose_offset);
        }
        m_eeprom.write(base_addr + portgroup_config_field::INPUT_TYPE_COUNT, config.input_types.size());

        u8 port_bitfield = 0;
        for (auto &port_number: config.output_port_numbers) {
            port_bitfield |= 0x80 >> port_number;
        }
        m_eeprom.write(base_addr + portgroup_config_field::OUTPUT_PORTS, port_bitfield);

        for (auto &input_type: config.input_types) {
            m_eeprom.write(base_addr + configuration_size, input_type);
            configuration_size++;
        }

        return configuration_size;
    }

    archive_parser::archive_parser(microwire_eeprom& eeprom)
        : k_eeprom(eeprom)
        , m_archive_size(0) {
        // nothing to do
    }

    archive_parser::~archive_parser() {
        // nothing to do
    }

    std::unique_ptr<const struct system_config> archive_parser::get_config() const {
        return std::make_unique<const struct system_config>(m_system_config);
    }

    const config_archive::operation_result archive_parser::read_header() {
        // get stored size
        m_archive_size = k_eeprom.read_2byte(static_header_field::SIZE0);
        if (m_archive_size < static_header_field::FIRST_CONFIG_BASE_ADDR) {
            return config_archive::operation_result::ARCHIVE_EMPTY;
        }
        if (m_archive_size > hw_setup.eeprom.size) {
            return config_archive::operation_result::SIZE_MISMATCH;
        }

        // get config counts
        u8 port_config_count = k_eeprom.read(static_header_field::PORT_CONFIG_COUNT);
        u8 portgroup_config_count = k_eeprom.read(static_header_field::PORTGROUP_CONFIG_COUNT);
        if (!port_config_count && !portgroup_config_count) {
            return config_archive::operation_result::CORRUPT_HEADER;
        }
        m_port_config_addrs.reserve(port_config_count);
        m_portgroup_config_addrs.reserve(portgroup_config_count);

        m_system_config.system_ports.reserve(port_config_count);
        m_system_config.system_port_groups.reserve(portgroup_config_count);

        // read out config base addresses
        for (u16 port_config_pointer = static_header_field::FIRST_CONFIG_BASE_ADDR;
            port_config_pointer < static_header_field::FIRST_CONFIG_BASE_ADDR + (port_config_count * 2); port_config_pointer += 2)
            {
            auto config_base_addr = k_eeprom.read_2byte(port_config_pointer);
            if (config_base_addr < port_config_pointer) {
                // illegal address, nope out and
                // return bad address failure
                return config_archive::operation_result::ILLEGAL_ADDRESS_ON_READ;
            }
            m_port_config_addrs.push_back(config_base_addr);
        }

        for (u16 portgroup_config_pointer = static_header_field::FIRST_CONFIG_BASE_ADDR + (port_config_count * 2);
            portgroup_config_pointer < static_header_field::FIRST_CONFIG_BASE_ADDR + ((port_config_count + portgroup_config_count) * 2);
            portgroup_config_pointer += 2)
            {
            auto config_base_addr = k_eeprom.read_2byte(portgroup_config_pointer);
            if (config_base_addr < portgroup_config_pointer) {
                // illegal address, nope out and
                // return bad address failure
                return config_archive::operation_result::ILLEGAL_ADDRESS_ON_READ;
            }
            m_portgroup_config_addrs.push_back(config_base_addr);
        }

        return config_archive::operation_result::SUCCESS;
    }

    archive_parser_v1::archive_parser_v1(microwire_eeprom& eeprom)
        : archive_parser(eeprom) {
        // nothing to do
    }

    archive_parser_v1::~archive_parser_v1() {
        // nothing to do
    }

    const config_archive::operation_result archive_parser_v1::parse() {
        auto last_result = read_header();
        if (last_result != config_archive::operation_result::SUCCESS) {
            return last_result;
        }

        // deserialise output_ports
        for (auto& port_config_addr: m_port_config_addrs) {
            m_system_config.system_ports.push_back(std::move(deserialise_port(port_config_addr)));
        }

        // deserialise portgroups
        u8 pg_id = 1;
        for (auto& portgroup_config_addr: m_portgroup_config_addrs) {
            m_system_config.system_port_groups.push_back(std::move(deserialise_portgroup(portgroup_config_addr, pg_id++)));
        }
        return last_result;
    }

    const struct output_port_config archive_parser_v1::deserialise_port(const u16 base_addr) const {
        const struct output_port_config port_config {
            .port_number {read_port_number(base_addr)},
            .clock_rate {read_port_clock_rate(base_addr)},
            .velocity_output {read_port_velocity(base_addr)}
        };
        return port_config;
    }

    const struct port_group_config archive_parser_v1::deserialise_portgroup(const u16 base_addr, const u8 pg_id) const {
        const struct port_group_config pg_config {
            .id {pg_id},
            .demux {read_portgroup_demux(base_addr)},
            .midi_channel {read_portgroup_chan(base_addr)},
            .cont_controller_number {read_portgroup_cc(base_addr)},
            .transpose_offset {read_portgroup_transpose(base_addr)},
            .input_types {read_portgroup_msg_types(base_addr)},
            .output_port_numbers {read_portgroup_ports(base_addr)}
        };
        return pg_config;
    }

    const u8 archive_parser_v1::read_port_number(const u16 base_addr) const {
        return k_eeprom.read(base_addr + port_config_field::PORT_NUMBER);
    }

    const u8 archive_parser_v1::read_port_clock_rate(const u16 base_addr) const {
        return k_eeprom.read(base_addr + port_config_field::CLOCK_RATE);
    }

    const bool archive_parser_v1::read_port_velocity(const u16 base_addr) const {
        return k_eeprom.read(base_addr + port_config_field::VELOCITY);
    }

    const demux_type archive_parser_v1::read_portgroup_demux(const u16 base_addr) const {
        // demux type value must be in range of enum type
        if (k_eeprom.read(base_addr + portgroup_config_field::DEMUX_TYPE) <= demux_type::FIFO) {
            return static_cast<const demux_type>(k_eeprom.read(base_addr + portgroup_config_field::DEMUX_TYPE));
        } else {
            return demux_type::RANDOM;
        }
    }

    const u8 archive_parser_v1::read_portgroup_chan(const u16 base_addr) const {
        return k_eeprom.read(base_addr + portgroup_config_field::MIDI_CHANNEL);
    }

    const u8 archive_parser_v1::read_portgroup_cc(const u16 base_addr) const {
        return k_eeprom.read(base_addr + portgroup_config_field::CC_NUMBER);
    }

    const i8 archive_parser_v1::read_portgroup_transpose(const u16 base_addr) const {
        auto transpose_value = k_eeprom.read(base_addr + portgroup_config_field::TRANSPOSE);
        if (transpose_value >> 7) {
            // if signage bit is set store as negative value
            return (-1) * ((i8) (transpose_value & 0x7f));
        } else {
            return transpose_value;
        }
    }

    const std::vector<midi_message::message_type> archive_parser_v1::read_portgroup_msg_types(const u16 base_addr) const {
        const u8 midi_input_count = k_eeprom.read(base_addr + portgroup_config_field::INPUT_TYPE_COUNT);

        std::vector<midi_message::message_type> msg_types;
        msg_types.reserve(midi_input_count);

        for (u16 midi_input_field = base_addr + portgroup_config_field::FIRST_VARIABLE;
            midi_input_field < (base_addr + portgroup_config_field::FIRST_VARIABLE + midi_input_count);
            midi_input_field++) {
            auto msg_type = k_eeprom.read(midi_input_field);
            // message type value must be in range of enum type
            if (msg_type > midi_message::STOP) {
                continue;
            }
            msg_types.push_back(static_cast<midi_message::message_type>(msg_type));
        }
        return msg_types;
    }

    const std::vector<u8> archive_parser_v1::read_portgroup_ports(const u16 base_addr) const {
        std::vector<u8> output_port_numbers;
        output_port_numbers.reserve(8);

        u8 outport_bitfield = k_eeprom.read(base_addr + portgroup_config_field::OUTPUT_PORTS);
        for (u8 outport = 0; outport < 8; outport++) {
            if (outport_bitfield & 0x80) {
                output_port_numbers.push_back(outport);
            }
            outport_bitfield <<= 1;
        }
        return output_port_numbers;
    }

    archive_parser_v2::archive_parser_v2(microwire_eeprom& eeprom)
        : archive_parser_v1(eeprom) {
        // nothing to do
    }

    archive_parser_v2::~archive_parser_v2() {
        // nothing to do
    }

    const struct output_port_config archive_parser_v2::deserialise_port(const u16 base_addr) const {
        const struct output_port_config port_config {
            .port_number {read_port_number(base_addr)},
            .clock_rate {read_port_clock_rate(base_addr)},
            .velocity_output {read_port_velocity(base_addr)},
            .clock_mode {read_port_clock_mode(base_addr)}
        };
        return port_config;
    }

    const output_port::clock_mode archive_parser_v2::read_port_clock_mode(const u16 base_addr) const {
        auto clock_mode = k_eeprom.read(base_addr + archive_parser_v2::port_config_field::CLOCK_MODE);

        if (clock_mode > output_port::clock_mode::SIGNAL_TRIGGER_STOP) {
            return output_port::clock_mode::SYNC;
        } else {
            return static_cast<const output_port::clock_mode>(clock_mode);
        }
    }
} // namespace midimagic
