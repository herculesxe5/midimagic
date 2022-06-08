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

#include "config_archive.h"

namespace midimagic {
    config_archive::config_archive()
        : m_eeprom(EEPROM_MOSI, EEPROM_MISO, EEPROM_CLK, EEPROM_CS, EEPROM_SIZE)
        , m_running_portgroup_id(0) {
        // nothing to do
    }

    config_archive::config_archive(const struct system_config system_state)
        : m_eeprom(EEPROM_MOSI, EEPROM_MISO, EEPROM_CLK, EEPROM_CS, EEPROM_SIZE)
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
        if (((m_eeprom.read(static_header_field::MAGIC0) << 8)
            + m_eeprom.read(static_header_field::MAGIC1)) != MAGIC) {
                // no joy, abort
                return operation_result::NO_ARCHIVE_FOUND;
        }

        // check version
        if (m_eeprom.read(static_header_field::VERSION) != RUNNING_VERSION) {
            return operation_result::VERSION_UNKNOWN;
        }

        // get stored size
        u16 total_size = m_eeprom.read(static_header_field::SIZE0) << 8;
        total_size += m_eeprom.read(static_header_field::SIZE1);
        if (total_size < static_header_field::FIRST_CONFIG_BASE_ADDR) {
            return operation_result::ARCHIVE_EMPTY;
        }
        if (total_size > EEPROM_SIZE) {
            return operation_result::SIZE_MISMATCH;
        }

        // get config counts
        u8 port_config_count = m_eeprom.read(static_header_field::PORT_CONFIG_COUNT);
        u8 portgroup_config_count = m_eeprom.read(static_header_field::PORTGROUP_CONFIG_COUNT);
        if (!port_config_count && !portgroup_config_count) {
            return operation_result::CORRUPT_HEADER;
        }

        // check if size matches
        u16 last_config_base, check_size;
        if (portgroup_config_count) {
            last_config_base = get_address_to(config_type::PORTGROUP_CONFIG, portgroup_config_count - 1);
            check_size = last_config_base + 1 + v1_portgroup_config_field::OUTPUT_PORTS;
            check_size += m_eeprom.read(last_config_base + v1_portgroup_config_field::INPUT_TYPE_COUNT);
        } else {
            last_config_base = get_address_to(config_type::OUTPUT_PORT_CONFIG, port_config_count - 1);
            check_size = last_config_base + 1 + v1_port_config_field::VELOCITY;
        }
        if (total_size != check_size) {
            return operation_result::SIZE_MISMATCH;
        }

        // populate m_system_state from the buffer

        // read output port configs
        for (u8 index = 0; index < port_config_count; index++) {
            u16 next_address = get_address_to(config_type::OUTPUT_PORT_CONFIG, index);
            operation_result op_state;
            if (next_address) {
                op_state = deserialise(config_type::OUTPUT_PORT_CONFIG, next_address);
            } else {
                return operation_result::CORRUPT_HEADER;
            }
            if (op_state != operation_result::SUCCESS) {
                return op_state;
            }
        }

        // read portgroup configs
        for (u8 index = 0; index < portgroup_config_count; index++) {
            u16 next_address = get_address_to(config_type::PORTGROUP_CONFIG, index);
            operation_result op_state;
            if (next_address) {
                op_state = deserialise(config_type::PORTGROUP_CONFIG, next_address);
            } else {
                return operation_result::CORRUPT_HEADER;
            }
            if (op_state != operation_result::SUCCESS) {
                return op_state;
            }
        }
        return operation_result::SUCCESS;
    }

    const config_archive::operation_result config_archive::writeout() {
        m_eeprom.enable_write();

        u16 header_size = generate_archive_header();

        // check if config fits
        u16 predicted_size = ((u16) m_eeprom.read(static_header_field::SIZE0)) << 8;
        predicted_size += m_eeprom.read(static_header_field::SIZE1);
        if (predicted_size > EEPROM_SIZE) {
            return operation_result::CONFIG_TOO_BIG;
            m_eeprom.disable_write();
        }

        u16 return_config_size;

        for (u8 index = 0; index < m_system_state.system_ports.size(); index++) {
            u16 next_address = get_address_to(config_type::OUTPUT_PORT_CONFIG, index);
            if (next_address) {
                return_config_size = serialise(m_system_state.system_ports.at(index), next_address);
            } else {
                return operation_result::ILLEGAL_ADDRESS_ON_WRITE;
                m_eeprom.disable_write();
            }
            if (!return_config_size) {
                return operation_result::ILLEGAL_CONFIG_BASE_ADDRESS;
                m_eeprom.disable_write();
            }
        }

        for (u8 index = 0; index < m_system_state.system_port_groups.size(); index++) {
            u16 next_address = get_address_to(config_type::PORTGROUP_CONFIG, index);
            if (next_address) {
                return_config_size = serialise(m_system_state.system_port_groups.at(index), next_address);
            } else {
                return operation_result::ILLEGAL_ADDRESS_ON_WRITE;
                m_eeprom.disable_write();
            }
            if (!return_config_size) {
                return operation_result::ILLEGAL_CONFIG_BASE_ADDRESS;
                m_eeprom.disable_write();
            }
        }
        m_eeprom.disable_write();
        return operation_result::SUCCESS;
    }

    u16 config_archive::generate_archive_header() {
        // write magic
        m_eeprom.write(static_header_field::MAGIC0, MAGIC);
        m_eeprom.write(static_header_field::MAGIC1, MAGIC);
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
            m_eeprom.write(running_header_field_addr, (u8) ((running_config_base_addr >> 8) & 0xff));
            m_eeprom.write(running_header_field_addr + 1, (u8) (running_config_base_addr & 0xff));
            running_config_base_addr += k_port_config_size;
            running_header_field_addr += 2;
        }

        // write portgroup config addresses
        for (auto &pg_config: m_system_state.system_port_groups) {
            m_eeprom.write(running_header_field_addr, (u8) ((running_config_base_addr >> 8) & 0xff));
            m_eeprom.write(running_header_field_addr + 1, (u8) (running_config_base_addr & 0xff));
            running_config_base_addr += k_fixed_portgroup_config_size + (pg_config.input_types.size());
            running_header_field_addr += 2;
        }

        // write total size
        // running_config_base_addr points to next next cell after config which is also the size of the config
        m_eeprom.write(static_header_field::SIZE0, (u8) (running_config_base_addr >> 8));
        m_eeprom.write(static_header_field::SIZE1, (u8) (running_config_base_addr & 0xff));

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
        out_addr += ((u16) m_eeprom.read(header_field_addr)) << 8;
        out_addr += ((u16) m_eeprom.read(header_field_addr + 1));
        return out_addr;
    }

    u16 config_archive::serialise(struct output_port_config config, u16 base_addr) {
        if (base_addr < static_header_field::FIRST_CONFIG_BASE_ADDR + 2) {
            // illegal address, no configs exist in this case, nope out...
            return 0;
        }
        m_eeprom.write(base_addr + v1_port_config_field::PORT_NUMBER, config.port_number);
        m_eeprom.write(base_addr + v1_port_config_field::CLOCK_RATE, config.clock_rate);
        m_eeprom.write(base_addr + v1_port_config_field::VELOCITY, config.velocity_output);
        return k_port_config_size;
    }

    u16 config_archive::serialise(struct port_group_config config, u16 base_addr) {
        if (base_addr < static_header_field::FIRST_CONFIG_BASE_ADDR + 2) {
            // illegal address, no configs exist in this case, nope out...
            return 0;
        }
        u16 configuration_size = k_fixed_portgroup_config_size;

        m_eeprom.write(base_addr + v1_portgroup_config_field::DEMUX_TYPE, config.demux);
        m_eeprom.write(base_addr + v1_portgroup_config_field::MIDI_CHANNEL, config.midi_channel);
        m_eeprom.write(base_addr + v1_portgroup_config_field::CC_NUMBER, config.cont_controller_number);
        if (config.transpose_offset < 0) {
            m_eeprom.write(base_addr + v1_portgroup_config_field::TRANSPOSE, (u8) (0x80 | (config.transpose_offset & 0x7f)));
        } else {
            m_eeprom.write(base_addr + v1_portgroup_config_field::TRANSPOSE, config.transpose_offset);
        }
        m_eeprom.write(base_addr + v1_portgroup_config_field::INPUT_TYPE_COUNT, config.input_types.size());

        u8 port_bitfield = 0;
        for (auto &port_number: config.output_port_numbers) {
            port_bitfield |= 0x80 >> port_number;
        }
        m_eeprom.write(base_addr + v1_portgroup_config_field::OUTPUT_PORTS, port_bitfield);

        for (auto &input_type: config.input_types) {
            m_eeprom.write(base_addr + configuration_size, input_type);
            configuration_size++;
        }

        return configuration_size;
    }

    const config_archive::operation_result config_archive::deserialise(config_type type, u16 base_addr) {
        if (base_addr < 6) { //FIXME 6 ?????????
            // illegal address, no configs exist in this case, nope out and
            // return bad address failure
            return operation_result::ILLEGAL_ADDRESS_ON_READ;
        }
        switch (type) {
            case config_type::OUTPUT_PORT_CONFIG :
                {
                struct output_port_config new_port;
                new_port.port_number = m_eeprom.read(base_addr);
                new_port.clock_rate = m_eeprom.read(base_addr + v1_port_config_field::CLOCK_RATE);
                new_port.velocity_output = m_eeprom.read(base_addr + v1_port_config_field::VELOCITY);
                m_system_state.system_ports.push_back(new_port);
                break;
                }
            case config_type::PORTGROUP_CONFIG :
                {
                struct port_group_config new_pg;
                new_pg.id = m_running_portgroup_id++;
                // demux type value must be in range of enum type
                if (m_eeprom.read(base_addr) <= demux_type::FIFO) {
                    new_pg.demux = static_cast<demux_type>(m_eeprom.read(base_addr));
                } else {
                    new_pg.demux = demux_type::RANDOM;
                }
                new_pg.midi_channel = m_eeprom.read(base_addr + v1_portgroup_config_field::MIDI_CHANNEL);
                new_pg.cont_controller_number = m_eeprom.read(base_addr + v1_portgroup_config_field::CC_NUMBER);
                i8 transpose_offset;
                if (m_eeprom.read(base_addr + v1_portgroup_config_field::TRANSPOSE) >> 7) {
                    // if signage bit is set store as negative value
                    transpose_offset = (-1) * ((i8) (m_eeprom.read(base_addr + v1_portgroup_config_field::TRANSPOSE) & 0x7f));
                } else {
                    transpose_offset = m_eeprom.read(base_addr + v1_portgroup_config_field::TRANSPOSE);
                }
                new_pg.transpose_offset = transpose_offset;
                const u8 midi_input_count = m_eeprom.read(base_addr + v1_portgroup_config_field::INPUT_TYPE_COUNT);

                u8 outport_bitfield = m_eeprom.read(base_addr + v1_portgroup_config_field::OUTPUT_PORTS);
                for (u8 outport = 0; outport < 8; outport++) {
                    if (outport_bitfield & 0x80) {
                        new_pg.output_port_numbers.push_back(outport);
                    }
                    outport_bitfield <<= 1;
                }

                for (u16 midi_input_field = base_addr + v1_portgroup_config_field::FIRST_VARIABLE;
                    midi_input_field < (base_addr + v1_portgroup_config_field::FIRST_VARIABLE + midi_input_count);
                    midi_input_field++) {
                    // message_type value must be in range of enum type
                    if (m_eeprom.read(midi_input_field) > midi_message::STOP) {
                        continue;
                    }
                    new_pg.input_types.push_back(static_cast<midi_message::message_type>(m_eeprom.read(midi_input_field)));
                }

                m_system_state.system_port_groups.push_back(new_pg);
                break;
                }
            default :
                return operation_result::UNKNOWN_CONFIG_TYPE_ON_READ;
                break;
        }
        return operation_result::SUCCESS;
    }
} // namespace midimagic
