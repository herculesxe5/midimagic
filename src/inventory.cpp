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

#include "inventory.h"
#include "common.h"
namespace midimagic {
    inventory::inventory(std::shared_ptr<group_dispatcher> gd,
                        std::shared_ptr<menu_action_queue> menu_q,
                        ad57x4 &dac0,
                        ad57x4 &dac1)
        : m_group_dispatcher(gd)
        , m_menu_q(menu_q)
        , m_dac0(dac0)
        , m_dac1(dac1) {
        // nothing to do
    }

    inventory::inventory(std::shared_ptr<group_dispatcher> gd,
                        std::shared_ptr<menu_action_queue> menu_q,
                        ad57x4 &dac0,
                        ad57x4 &dac1,
                        const struct system_config& init_config)
        : m_group_dispatcher(gd)
        , m_menu_q(menu_q)
        , m_dac0(dac0)
        , m_dac1(dac1) {
        apply_config(init_config);
    }

    inventory::~inventory() {
        // nothing to do
    }

    std::shared_ptr<output_port> inventory::get_output_port(const u8 port_number) {
        for (auto &port: m_system_ports) {
            if (port->get_port_number() == port_number) {
                return port;
            }
        }
        spawn_port(port_number);
        auto new_port = m_system_ports.back();
        struct output_port_config new_port_config;
        new_port_config.port_number = port_number;
        new_port_config.clock_rate = new_port->get_clock_rate();
        new_port_config.velocity_output = new_port->get_velocity_switch();
        m_system_config.system_ports.push_back(new_port_config);
        return new_port;
    }

    std::shared_ptr<group_dispatcher> inventory::get_group_dispatcher() {
        return m_group_dispatcher;
    }

    std::shared_ptr<menu_action_queue> inventory::get_menu_queue() {
        return m_menu_q;
    }

    void inventory::apply_config(const struct system_config& new_config) {

        flush();
        // overwrite old system_config with new_config
        m_system_config = sanitise_config(new_config);
        // setup output ports
        std::shared_ptr<output_port> system_port;
        for (auto &port_config: m_system_config.system_ports) {
            if (port_exists(port_config.port_number)) {
                system_port = get_output_port(port_config.port_number);
            } else {
                // dont use get_output_port if port is non-existent to avoid overwriting the new config values
                spawn_port(port_config.port_number);
                system_port = m_system_ports.back();
            }
            system_port->set_clock_rate(port_config.clock_rate);
            if (system_port->get_velocity_switch() != port_config.velocity_output) {
                system_port->set_velocity_switch();
            }
        }
        // setup port groups
        for (auto &pg_config: m_system_config.system_port_groups) {
            u8 new_pg_id = spawn_port_group(config_pg_it_by_id(pg_config.id));
        }
    }

    config_archive::operation_result inventory::load_config_from_eeprom() {
        config_archive eeprom_config;
        config_archive::operation_result load_result = eeprom_config.loadon();
        if (load_result == config_archive::operation_result::SUCCESS) {
            apply_config(eeprom_config.spellout());
        }
        return load_result;
    }

    config_archive::operation_result inventory::save_system_state() {
        auto system_state = gather_system_state();
        config_archive new_eeprom_config(*system_state);
        //config_archive new_eeprom_config(m_system_config);
        return new_eeprom_config.writeout();
    }

    void inventory::flush() {
        auto& pg_vector = m_group_dispatcher->get_port_groups();
        while (!pg_vector.empty()) {
            m_group_dispatcher->remove_port_group((pg_vector.back())->get_id());
        }
        m_system_config.system_port_groups.clear();
    }

    std::unique_ptr<const struct system_config> inventory::gather_system_state() const {
        struct system_config current_state;

        // generate output_port_configs
        for (auto& port: m_system_ports) {
            const struct output_port_config current_port = {port->get_port_number(), port->get_clock_rate(), port->get_velocity_switch()};
            current_state.system_ports.push_back(std::move(current_port));
        }

        // generate port_group_configs
        auto& port_groups = m_group_dispatcher->get_port_groups();
        for (auto& port_group: port_groups) {
            struct port_group_config current_pg = {
            port_group->get_id(),
            port_group->get_demux().get_type(),
            port_group->get_midi_channel(),
            port_group->get_cc(),
            port_group->get_transpose()
            };

            // the message input types vector can just be copied
            current_pg.input_types.reserve(port_group->get_msg_types().size());
            std::copy(port_group->get_msg_types().begin(), port_group->get_msg_types().end(), std::back_inserter(current_pg.input_types));

            // the output_port ids must be read out of the objects directly
            auto& ports_vector = port_group->get_demux().get_output();
            for (auto& port: ports_vector) {
                current_pg.output_port_numbers.push_back(port->get_port_number());
            }

            current_state.system_port_groups.push_back(std::move(current_pg));
        }

        return std::make_unique<const struct system_config>(current_state);
    }

    void inventory::spawn_port(const u8 config_port_number) {
        ad57x4::dac_channel dac_ch;
        if (config_port_number < 4) {
            dac_ch = static_cast<ad57x4::dac_channel>(config_port_number);
            m_system_ports.push_back(std::make_shared<output_port>(
                port_number2digital_pin[config_port_number],
                dac_ch,
                m_dac0,
                m_menu_q,
                config_port_number));
        } else {
            dac_ch = static_cast<ad57x4::dac_channel>(config_port_number - 4);
            m_system_ports.push_back(std::make_shared<output_port>(
                port_number2digital_pin[config_port_number],
                dac_ch,
                m_dac1,
                m_menu_q,
                config_port_number));
        }
    }

    const u8 inventory::spawn_port_group(std::vector<struct port_group_config>::iterator config_pg_it) {
        m_group_dispatcher->add_port_group(config_pg_it->demux, config_pg_it->midi_channel);
        auto& new_pg = (m_group_dispatcher->get_port_groups()).back();
        // set cc number
        new_pg->set_cc(config_pg_it->cont_controller_number);
        // set transpose
        new_pg->set_transpose(config_pg_it->transpose_offset);
        // add the midi inputs
        for (auto &msg_type: config_pg_it->input_types) {
            new_pg->add_midi_input(msg_type);
        }
        // assign output_ports
        for (auto &portnumber_in_config: config_pg_it->output_port_numbers) {
            for (auto &system_port: m_system_ports) {
                if (portnumber_in_config == system_port->get_port_number()) {
                    new_pg->add_port(system_port);
                }
            }
        }
        return new_pg->get_id();
    }

    std::vector<struct port_group_config>::iterator inventory::config_pg_it_by_id(u8 config_pg_id) {
        for (auto it = m_system_config.system_port_groups.begin();
            it != m_system_config.system_port_groups.end(); ) {
            if (it->id == config_pg_id) {
                return it;
            }
            ++it;
        }
    }

    std::vector<struct output_port_config>::iterator inventory::config_port_it_by_number(u8 config_port_number) {
        for (auto it = m_system_config.system_ports.begin();
            it != m_system_config.system_ports.end(); ) {
            if (it->port_number == config_port_number) {
                return it;
            }
            ++it;
        }
    }



    const std::vector<std::unique_ptr<port_group>>::const_iterator inventory::system_pg_it_by_id(const u8 system_pg_id) const {
        for (auto it = m_group_dispatcher->get_port_groups().begin();
            it != m_group_dispatcher->get_port_groups().end(); ) {
            if ((*it)->get_id() == system_pg_id) {
                return it;
            }
            ++it;
        }
    }

    const std::vector<std::shared_ptr<output_port>>::const_iterator inventory::system_port_it_by_number(const u8 system_port_number) const {
        for (auto it = m_system_ports.begin(); it != m_system_ports.end(); ) {
            if ((*it)->get_port_number() == system_port_number) {
                return it;
            }
            ++it;
        }
    }

    const bool inventory::port_exists(const u8 port_number) const {
        for (auto &system_port: m_system_ports) {
            if (system_port->get_port_number() == port_number) {
                return true;
            }
        }
        return false;
    }

    const struct system_config inventory::sanitise_config(const struct system_config in_config) const {
        // FIXME add flag if change happend
        struct system_config out_config;
        // check for unique output port numbers and numbers in range (0...7)
        for (auto it = in_config.system_ports.begin(); it != in_config.system_ports.end(); ) {
            if (it->port_number > 7) {
                // ignore this output port
                ++it;
                continue;
            }

            bool duplicate_port = false;

            for (auto next_it = std::next(it, 1); next_it != in_config.system_ports.end(); ) {
                // if a following output port config has the same port number ignore current
                if (next_it->port_number == it->port_number) {
                    duplicate_port = true;
                    break;
                }
                ++next_it;
            }
            if (duplicate_port) {
                ++it;
                continue;
            }
            // all good, copy config into out struct and move on
            out_config.system_ports.emplace_back(*it);
            ++it;
        }

        u8 next_free_id = 0;
        // get highest id in vector
        for (auto it = in_config.system_port_groups.begin(); it != in_config.system_port_groups.end(); ) {
            if (it->id > next_free_id) {
                next_free_id = it->id;
            }
            ++it;
        }
        next_free_id++;

        for (auto it = in_config.system_port_groups.begin(); it != in_config.system_port_groups.end(); ) {
            out_config.system_port_groups.emplace_back(*it);
            // check for unique portgroup ids
            for (auto next_it = std::next(it, 1); next_it != in_config.system_port_groups.end(); ) {
                // if a following portgroup config has the same id assign next free to current
                if (next_it->id == it->id) {
                    out_config.system_port_groups.back().id = next_free_id;
                    next_free_id++;
                    break;
                }
                ++next_it;
            }
            // check midi channel in range of (1...16)
            if ((it->midi_channel < 1) || (it->midi_channel > 16)) {
                out_config.system_port_groups.back().midi_channel = 1;
            }
            // cc value must be in range of (0...127)
            if (it->cont_controller_number > 127) {
                out_config.system_port_groups.back().cont_controller_number = 127;
            }

            ++it;
        }
        return out_config;
    }

} // namespace midimagic