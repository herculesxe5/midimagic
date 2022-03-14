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
        : m_running_portgroup_id(0) {
        // nothing to do
    }

    config_archive::config_archive(const struct system_config system_state)
        : m_running_portgroup_id(0) {
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
        // load flash content to the eeprom buffer
        load_from_flash();

        // check for magic
        if (((read_eeprom_buffer(static_header_field::MAGIC0) << 8)
            + read_eeprom_buffer(static_header_field::MAGIC1)) != MAGIC) {
                // no joy, abort
                return operation_result::NO_ARCHIVE_FOUND;
        }

        // check version
        if (read_eeprom_buffer(static_header_field::VERSION) != RUNNING_VERSION) {
            return operation_result::VERSION_UNKNOWN;
        }

        // get stored size
        u16 total_size = read_eeprom_buffer(static_header_field::SIZE0) << 8;
        total_size += read_eeprom_buffer(static_header_field::SIZE1);
        if (total_size < static_header_field::FIRST_CONFIG_BASE_ADDR) {
            return operation_result::ARCHIVE_EMPTY;
        }
        if (total_size > BUFFER_SIZE) {
            return operation_result::SIZE_MISMATCH;
        }

        // get config counts
        u8 port_config_count = read_eeprom_buffer(static_header_field::PORT_CONFIG_COUNT);
        u8 portgroup_config_count = read_eeprom_buffer(static_header_field::PORTGROUP_CONFIG_COUNT);
        if (!port_config_count && !portgroup_config_count) {
            return operation_result::CORRUPT_HEADER;
        }

        // check if size matches
        uint32_t last_config_base, check_size;
        if (portgroup_config_count) {
            last_config_base = get_address_to(config_type::PORTGROUP_CONFIG, portgroup_config_count - 1);
            check_size = last_config_base + 1 + v1_portgroup_config_field::OUTPUT_PORTS;
            check_size += read_eeprom_buffer(last_config_base + v1_portgroup_config_field::INPUT_TYPE_COUNT);
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
            uint32_t next_address = get_address_to(config_type::OUTPUT_PORT_CONFIG, index);
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
            uint32_t next_address = get_address_to(config_type::PORTGROUP_CONFIG, index);
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
        uint32_t header_size = generate_archive_header();

        // check if config fits
        u16 predicted_size = ((u16) read_eeprom_buffer(static_header_field::SIZE0)) << 8;
        predicted_size += read_eeprom_buffer(static_header_field::SIZE1);
        if (predicted_size > BUFFER_SIZE) {
            return operation_result::CONFIG_TOO_BIG;
        }

        uint32_t return_config_size;

        for (u8 index = 0; index < m_system_state.system_ports.size(); index++) {
            uint32_t next_address = get_address_to(config_type::OUTPUT_PORT_CONFIG, index);
            if (next_address) {
                return_config_size = serialise(m_system_state.system_ports.at(index), next_address);
            } else {
                return operation_result::ILLEGAL_ADDRESS_ON_WRITE;
            }
            if (!return_config_size) {
                return operation_result::ILLEGAL_CONFIG_BASE_ADDRESS;
            }
        }

        for (u8 index = 0; index < m_system_state.system_port_groups.size(); index++) {
            uint32_t next_address = get_address_to(config_type::PORTGROUP_CONFIG, index);
            if (next_address) {
                return_config_size = serialise(m_system_state.system_port_groups.at(index), next_address);
            } else {
                return operation_result::ILLEGAL_ADDRESS_ON_WRITE;
            }
            if (!return_config_size) {
                return operation_result::ILLEGAL_CONFIG_BASE_ADDRESS;
            }
        }

        write_to_flash();
        return operation_result::SUCCESS;
    }

    uint32_t config_archive::generate_archive_header() {
        // write magic
        write_eeprom_buffer(static_header_field::MAGIC0, MAGIC);
        write_eeprom_buffer(static_header_field::MAGIC1, MAGIC);
        // write version
        write_eeprom_buffer(static_header_field::VERSION, RUNNING_VERSION);

        // write config counts
        u8 port_config_count = m_system_state.system_ports.size();
        u8 portgroup_config_count = m_system_state.system_port_groups.size();
        write_eeprom_buffer(static_header_field::PORT_CONFIG_COUNT, port_config_count);
        write_eeprom_buffer(static_header_field::PORTGROUP_CONFIG_COUNT, portgroup_config_count);

        // calculate addresses
        uint32_t header_size = static_header_field::FIRST_CONFIG_BASE_ADDR + (4 * port_config_count) + (4 * portgroup_config_count);

        uint32_t running_config_base_addr = header_size;
        uint32_t running_header_field_addr = static_header_field::FIRST_CONFIG_BASE_ADDR;

        // write port config addresses
        for (u8 i = 0; i < port_config_count; i++) {
            write_eeprom_buffer(running_header_field_addr, (u8) running_config_base_addr >> 24);
            write_eeprom_buffer(running_header_field_addr + 1, (u8) ((running_config_base_addr >> 16) & 0xff));
            write_eeprom_buffer(running_header_field_addr + 2, (u8) ((running_config_base_addr >> 8) & 0xff));
            write_eeprom_buffer(running_header_field_addr + 3, (u8) (running_config_base_addr & 0xff));
            running_config_base_addr += k_port_config_size;
            running_header_field_addr += 4;
        }

        // write portgroup config addresses
        for (auto &pg_config: m_system_state.system_port_groups) {
            write_eeprom_buffer(running_header_field_addr, (u8) running_config_base_addr >> 24);
            write_eeprom_buffer(running_header_field_addr + 1, (u8) ((running_config_base_addr >> 16) & 0xff));
            write_eeprom_buffer(running_header_field_addr + 2, (u8) ((running_config_base_addr >> 8) & 0xff));
            write_eeprom_buffer(running_header_field_addr + 3, (u8) (running_config_base_addr & 0xff));
            running_config_base_addr += k_fixed_portgroup_config_size + (pg_config.input_types.size());
            running_header_field_addr += 4;
        }

        // write total size
        // running_config_base_addr points to next next cell after config which is also the size of the config
        write_eeprom_buffer(static_header_field::SIZE0, (u8) (running_config_base_addr >> 8));
        write_eeprom_buffer(static_header_field::SIZE1, (u8) (running_config_base_addr & 0xff));

        return header_size;
    }

    uint32_t config_archive::get_address_to(config_type type, u8 index) {
        u8 port_config_count = read_eeprom_buffer(static_header_field::PORT_CONFIG_COUNT);
        u8 portgroup_config_count = read_eeprom_buffer(static_header_field::PORTGROUP_CONFIG_COUNT);
        uint32_t out_addr = 0;
        uint32_t header_field_addr = 0;
        switch (type) {
            case config_type::OUTPUT_PORT_CONFIG :
                if (index >= port_config_count) {
                    return out_addr;
                }
                header_field_addr = static_header_field::FIRST_CONFIG_BASE_ADDR + (index * 4);
                break;
            case config_type::PORTGROUP_CONFIG :
                if (index >= portgroup_config_count) {
                    return out_addr;
                }
                header_field_addr = static_header_field::FIRST_CONFIG_BASE_ADDR + (port_config_count * 4) + (index * 4);
                break;
            default :
                return out_addr;
                break;
        }
        out_addr += ((uint32_t) read_eeprom_buffer(header_field_addr)) << 24;
        out_addr += ((uint32_t) read_eeprom_buffer(header_field_addr + 1)) << 16;
        out_addr += ((uint32_t) read_eeprom_buffer(header_field_addr + 2)) << 8;
        out_addr += ((uint32_t) read_eeprom_buffer(header_field_addr + 3));
        return out_addr;
    }

    uint32_t config_archive::serialise(struct output_port_config config, uint32_t base_addr) {
        if (base_addr < static_header_field::FIRST_CONFIG_BASE_ADDR + 4) {
            // illegal address, no configs exist in this case, nope out...
            return 0;
        }
        write_eeprom_buffer(base_addr + v1_port_config_field::PORT_NUMBER, config.port_number);
        write_eeprom_buffer(base_addr + v1_port_config_field::CLOCK_RATE, config.clock_rate);
        write_eeprom_buffer(base_addr + v1_port_config_field::VELOCITY, config.velocity_output);
        return k_port_config_size;
    }

    uint32_t config_archive::serialise(struct port_group_config config, uint32_t base_addr) {
        if (base_addr < static_header_field::FIRST_CONFIG_BASE_ADDR + 4) {
            // illegal address, no configs exist in this case, nope out...
            return 0;
        }
        uint32_t configuration_size = k_fixed_portgroup_config_size;

        write_eeprom_buffer(base_addr + v1_portgroup_config_field::DEMUX_TYPE, config.demux);
        write_eeprom_buffer(base_addr + v1_portgroup_config_field::MIDI_CHANNEL, config.midi_channel);
        write_eeprom_buffer(base_addr + v1_portgroup_config_field::CC_NUMBER, config.cont_controller_number);
        if (config.transpose_offset < 0) {
            write_eeprom_buffer(base_addr + v1_portgroup_config_field::TRANSPOSE, (u8) (0x80 | (config.transpose_offset & 0x7f)));
        } else {
            write_eeprom_buffer(base_addr + v1_portgroup_config_field::TRANSPOSE, config.transpose_offset);
        }
        write_eeprom_buffer(base_addr + v1_portgroup_config_field::INPUT_TYPE_COUNT, config.input_types.size());

        u8 port_bitfield = 0;
        for (auto &port_number: config.output_port_numbers) {
            port_bitfield |= 0x80 >> port_number;
        }
        write_eeprom_buffer(base_addr + v1_portgroup_config_field::OUTPUT_PORTS, port_bitfield);

        for (auto &input_type: config.input_types) {
            write_eeprom_buffer(base_addr + configuration_size, input_type);
            configuration_size++;
        }

        return configuration_size;
    }

    const config_archive::operation_result config_archive::deserialise(config_type type, uint32_t base_addr) {
        if (base_addr < 6) {
            // illegal address, no configs exist in this case, nope out and
            // return bad address failure
            return operation_result::ILLEGAL_ADDRESS_ON_READ;
        }
        switch (type) {
            case config_type::OUTPUT_PORT_CONFIG :
                {
                struct output_port_config new_port;
                new_port.port_number = read_eeprom_buffer(base_addr);
                new_port.clock_rate = read_eeprom_buffer(base_addr + v1_port_config_field::CLOCK_RATE);
                new_port.velocity_output = read_eeprom_buffer(base_addr + v1_port_config_field::VELOCITY);
                m_system_state.system_ports.push_back(new_port);
                break;
                }
            case config_type::PORTGROUP_CONFIG :
                {
                struct port_group_config new_pg;
                new_pg.id = m_running_portgroup_id++;
                // demux type value must be in range of enum type
                if (read_eeprom_buffer(base_addr) <= demux_type::FIFO) {
                    new_pg.demux = static_cast<demux_type>(read_eeprom_buffer(base_addr));
                } else {
                    new_pg.demux = demux_type::RANDOM;
                }
                new_pg.midi_channel = read_eeprom_buffer(base_addr + v1_portgroup_config_field::MIDI_CHANNEL);
                new_pg.cont_controller_number = read_eeprom_buffer(base_addr + v1_portgroup_config_field::CC_NUMBER);
                i8 transpose_offset;
                if (read_eeprom_buffer(base_addr + v1_portgroup_config_field::TRANSPOSE) >> 7) {
                    // if signage bit is set store as negative value
                    transpose_offset = (-1) * ((i8) (read_eeprom_buffer(base_addr + v1_portgroup_config_field::TRANSPOSE) & 0x7f));
                } else {
                    transpose_offset = read_eeprom_buffer(base_addr + v1_portgroup_config_field::TRANSPOSE);
                }
                new_pg.transpose_offset = transpose_offset;
                const u8 midi_input_count = read_eeprom_buffer(base_addr + v1_portgroup_config_field::INPUT_TYPE_COUNT);

                u8 outport_bitfield = read_eeprom_buffer(base_addr + v1_portgroup_config_field::OUTPUT_PORTS);
                for (u8 outport = 0; outport < 8; outport++) {
                    if (outport_bitfield & 0x80) {
                        new_pg.output_port_numbers.push_back(outport);
                    }
                    outport_bitfield <<= 1;
                }

                for (uint32_t midi_input_field = base_addr + v1_portgroup_config_field::FIRST_VARIABLE;
                    midi_input_field < (base_addr + v1_portgroup_config_field::FIRST_VARIABLE + midi_input_count);
                    midi_input_field++) {
                    // message_type value must be in range of enum type
                    if (read_eeprom_buffer(midi_input_field) > midi_message::STOP) {
                        continue;
                    }
                    new_pg.input_types.push_back(static_cast<midi_message::message_type>(read_eeprom_buffer(midi_input_field)));
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

    void config_archive::write_to_flash() {
        eeprom_buffer_flush();
    }

    void config_archive::load_from_flash() {
        eeprom_buffer_fill();
    }

    uint8_t read_eeprom_buffer(const uint32_t pos) {
        return eeprom_buffered_read_byte(pos);
    }

    void write_eeprom_buffer(uint32_t pos, uint8_t value) {
        eeprom_buffered_write_byte(pos, value);
    }

} // namespace midimagic
