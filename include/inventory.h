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

#ifndef MIDIMAGIC_INVENTORY_H
#define MIDIMAGIC_INVENTORY_H

#include "common.h"
#include "system_config.h"
#include "port_group.h"
#include "output.h"
#include "ad57x4.h"
#include "midi_types.h"
#include "config_archive.h"

namespace midimagic {
    class inventory {
    public:
        inventory(std::shared_ptr<group_dispatcher> gd,
                  std::shared_ptr<menu_action_queue> menu_q,
                  ad57x4 &dac0,
                  ad57x4 &dac1);
        inventory(std::shared_ptr<group_dispatcher> gd,
                  std::shared_ptr<menu_action_queue> menu_q,
                  ad57x4 &dac0,
                  ad57x4 &dac1,
                  const struct system_config& init_config);
        inventory() = delete;
        inventory(const inventory&) = delete;
        ~inventory();

        std::shared_ptr<output_port> get_output_port(const u8 port_number); // returns pointer to output_port by port number, creates object if needed
        std::shared_ptr<group_dispatcher> get_group_dispatcher(); // returns pointer to the system port group dispatcher
        std::shared_ptr<menu_action_queue> get_menu_queue();

        void apply_config(const struct system_config& new_config); // setup system as in new_config
        config_archive::operation_result load_config_from_eeprom();
        config_archive::operation_result save_system_state();

        const u8 port_number2digital_pin[8] {
            hw_setup.ports.dpin_port0,
            hw_setup.ports.dpin_port1,
            hw_setup.ports.dpin_port2,
            hw_setup.ports.dpin_port3,
            hw_setup.ports.dpin_port4,
            hw_setup.ports.dpin_port5,
            hw_setup.ports.dpin_port6,
            hw_setup.ports.dpin_port7
        };

    private:
        std::shared_ptr<group_dispatcher> m_group_dispatcher;
        std::shared_ptr<menu_action_queue> m_menu_q;
        ad57x4 &m_dac0, &m_dac1;
        struct system_config m_system_config;
        std::vector<std::shared_ptr<output_port>> m_system_ports;

        // destroys all port groups and deletes from system_config
        void flush();
        // creates new system_config from currrent system state
        std::unique_ptr<const struct system_config> gather_system_state() const;

        // creates output_port in m_system_ports
        void spawn_port(const u8 config_port_number);
        // creates new port group from system_config, returns new id
        const u8 spawn_port_group(std::vector<struct port_group_config>::iterator config_pg_it);

        std::vector<struct port_group_config>::iterator config_pg_it_by_id(u8 config_pg_id);
        std::vector<struct output_port_config>::iterator config_port_it_by_number(u8 config_port_number);
        const std::vector<std::unique_ptr<port_group>>::const_iterator system_pg_it_by_id(const u8 system_pg_id) const;
        const std::vector<std::shared_ptr<output_port>>::const_iterator system_port_it_by_number(const u8 system_port_number) const;
        const bool port_exists(const u8 port_number) const;
        const struct system_config sanitise_config(const struct system_config in_config) const;
    };
} // namespace midimagic
#endif // MIDIMAGIC_INVENTORY_H