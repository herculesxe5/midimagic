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

    void inventory::submit_portgroup_change(const u8 system_pg_id) {
        rescan_port_group(system_pg_it_by_id(system_pg_id));
    }

    void inventory::submit_portgroup_delete(const u8 system_pg_id) {
        forget_port_group(system_pg_it_by_id(system_pg_id));
    }

    void inventory::submit_portgroup_add(const u8 system_pg_id) {
        add_port_group(system_pg_it_by_id(system_pg_id));
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

    void inventory::submit_output_port_change(const u8 system_port_number) {
        if (port_exists(system_port_number)) {
            rescan_output_port(system_port_it_by_number(system_port_number));
        }
    }

    std::shared_ptr<group_dispatcher> inventory::get_group_dispatcher() {
        return m_group_dispatcher;
    }

    std::shared_ptr<menu_action_queue> inventory::get_menu_queue() {
        return m_menu_q;
    }

    void inventory::apply_config(const struct system_config& new_config) {
        //FIXME check config for sane values
        flush();
        //overwrite old system_config with new_config
        m_system_config = new_config;
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
        // iterate through the created system port groups and update ids in system_config
        auto& system_portgroups = m_group_dispatcher->get_port_groups();
        for (u8 i = 0; i < system_portgroups.size(); i++) {
            m_system_config.system_port_groups.at(i).id = system_portgroups.at(i)->get_id();
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
        config_archive new_eeprom_config(m_system_config);
        return new_eeprom_config.writeout();
    }

    void inventory::flush() {
        auto& pg_vector = m_group_dispatcher->get_port_groups();
        while (!pg_vector.empty()) {
            m_group_dispatcher->remove_port_group((pg_vector.back())->get_id());
        }
        m_system_config.system_port_groups.clear();
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

    void inventory::rescan_port_group(const std::vector<std::unique_ptr<port_group>>::const_iterator system_pg_it) {
        auto config_pg_it = config_pg_it_by_id((*system_pg_it)->get_id());
        config_pg_it->demux = ((*system_pg_it)->get_demux()).get_type();
        config_pg_it->midi_channel = (*system_pg_it)->get_midi_channel();
        config_pg_it->cont_controller_number = (*system_pg_it)->get_cc();
        config_pg_it->transpose_offset = (*system_pg_it)->get_transpose();
        config_pg_it->input_types.clear();
        for (auto &system_pg_input: (*system_pg_it)->get_msg_types()) {
            config_pg_it->input_types.push_back(system_pg_input);
        }
        config_pg_it->output_port_numbers.clear();
        for (auto &system_pg_port: ((*system_pg_it)->get_demux()).get_output()) {
            config_pg_it->output_port_numbers.push_back(system_pg_port->get_port_number());
        }
    }

    void inventory::add_port_group(const std::vector<std::unique_ptr<port_group>>::const_iterator system_pg_it) {
        struct port_group_config new_pg_config;
        new_pg_config.id = (*system_pg_it)->get_id();
        m_system_config.system_port_groups.push_back(new_pg_config);
        rescan_port_group(system_pg_it);
    }

    void inventory::forget_port_group(const std::vector<std::unique_ptr<port_group>>::const_iterator system_pg_it) {
        m_system_config.system_port_groups.erase(config_pg_it_by_id((*system_pg_it)->get_id()));
    }

    void inventory::rescan_output_port(const std::vector<std::shared_ptr<output_port>>::const_iterator system_port_it) {
        auto port_config = config_port_it_by_number((*system_port_it)->get_port_number());
        port_config->clock_rate = (*system_port_it)->get_clock_rate();
        port_config->velocity_output = (*system_port_it)->get_velocity_switch();
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

    const struct system_config inventory::sanitise_config(const struct system_config in_config) {
        // FIXME add flag if change happend
        struct system_config out_config;
        // check for unique output port numbers and numbers in range (0...7)
        for (auto it = in_config.system_ports.begin(); it != in_config.system_ports.end(); ) {
            if (it->port_number > 7) {
                ++it;
                continue;
            }
            for (auto next_it = std::next(it, 1); next_it != in_config.system_ports.end(); ) {
                // if a following output port config has the same port number ignore current
                if (next_it->port_number == it->port_number) {
                    ++it;
                    break;
                }
                ++next_it;
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
            // each output port number in output_port_numbers vector must be in range (0...7)
            // and have corresponding output port config
            for (auto outport_it = it->output_port_numbers.begin(); outport_it != it->output_port_numbers.end(); ) {
                if ((*outport_it) > 7) {
                    // find element in out_config and erase
                    for (auto outconfig_portnumber_it = out_config.system_port_groups.back().output_port_numbers.begin();
                    outconfig_portnumber_it != out_config.system_port_groups.back().output_port_numbers.end(); ) {
                        if ((*outconfig_portnumber_it) == (*outport_it)) {
                            out_config.system_port_groups.back().output_port_numbers.erase(outconfig_portnumber_it);
                            break;
                        }
                        ++outconfig_portnumber_it;
                    }
                    ++outport_it;
                    continue;
                }
                //FIXME add config exists check
                ++outport_it;
            }

            ++it;
        }
        return out_config;
    }

} // namespace midimagic