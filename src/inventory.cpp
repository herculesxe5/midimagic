#include "inventory.h"
#include "common.h"
namespace midimagic {
    inventory::inventory(group_dispatcher& gd,
                        std::shared_ptr<menu_action_queue> menu_q,
                        ad57x4 &dac0,
                        ad57x4 &dac1)
        : m_group_dispatcher(gd)
        , m_menu_q(menu_q)
        , m_dac0(dac0)
        , m_dac1(dac1) {
        // nothing to do
    }

    inventory::inventory(group_dispatcher& gd,
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

    const output_port& inventory::get_output_port(const u8 port_number) {
        for (auto &port: m_system_ports) {
            if (port.get_port_number() == port_number) {
                return port;
            }
        }
        spawn_port(port_number);
        return m_system_ports.back();
    }

    void inventory::apply_config(const struct system_config& new_config) {
        flush();
        //overwrite old system_config with new_config
        m_system_config = new_config;
        // setup output ports
        for (auto &port_config: m_system_config.system_ports) {
            if (!port_exists(port_config.port_number)) {
                spawn_port(port_config.port_number);
            }
        }
        // setup port groups
        for (auto &pg_config: m_system_config.system_port_groups) {
            u8 new_pg_id = spawn_port_group(config_pg_it_by_id(pg_config.id));
        }
    }

    void inventory::flush() {
        auto& pg_vector = m_group_dispatcher.get_port_groups();
        while (!pg_vector.empty()) {
            m_group_dispatcher.remove_port_group((pg_vector.back())->get_id());
        }
        m_system_config.system_port_groups.clear();
    }



    void inventory::spawn_port(const u8 config_port_number) {
        ad57x4::dac_channel dac_ch;
        if (config_port_number < 4) {
            dac_ch = static_cast<ad57x4::dac_channel>(config_port_number);
            m_system_ports.emplace_back(
                port_number2digital_pin[config_port_number],
                dac_ch,
                m_dac0,
                m_menu_q,
                config_port_number);
        } else {
            dac_ch = static_cast<ad57x4::dac_channel>(config_port_number - 4);
            m_system_ports.emplace_back(
                port_number2digital_pin[config_port_number],
                dac_ch,
                m_dac1,
                m_menu_q,
                config_port_number);
        }
    }

    const u8 inventory::spawn_port_group(std::vector<struct port_group_config>::iterator config_pg_it) {
        m_group_dispatcher.add_port_group(config_pg_it->demux, config_pg_it->midi_channel);
        auto& new_pg = (m_group_dispatcher.get_port_groups()).back();
        // add the midi inputs
        for (auto &msg_type: config_pg_it->input_types) {
            new_pg->add_midi_input(msg_type);
        }
        // assign output_ports
        for (auto &portnumber_in_config: config_pg_it->output_port_numbers) {
            for (auto &system_port: m_system_ports) {
                if (portnumber_in_config == system_port.get_port_number()) {
                    new_pg->add_port(system_port);
                }
            }
        }
        // update the newly assigned id in system_config to reflect the system setup
        config_pg_it->id = new_pg->get_id();
        return config_pg_it->id;
    }

    void inventory::rescan_port_group(const std::vector<std::unique_ptr<port_group>>::const_iterator system_pg_it) {
        auto config_pg_it = config_pg_it_by_id((*system_pg_it)->get_id());
        config_pg_it->demux = ((*system_pg_it)->get_demux()).get_type();
        config_pg_it->midi_channel = (*system_pg_it)->get_midi_channel();
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

    std::vector<struct inventory::port_group_config>::iterator inventory::config_pg_it_by_id(u8 config_pg_id) {
        for (auto it = m_system_config.system_port_groups.begin();
            it != m_system_config.system_port_groups.end(); ) {
            if (it->id == config_pg_id) {
                return it;
            }

        }
    }

    const std::vector<std::unique_ptr<port_group>>::const_iterator inventory::system_pg_it_by_id(const u8 system_pg_id) const {
        for (auto it = m_group_dispatcher.get_port_groups().begin();
            it != m_group_dispatcher.get_port_groups().end(); ) {
            if ((*it)->get_id() == system_pg_id) {
                return it;
            }
        }
    }

    const bool inventory::port_exists(const u8 port_number) const {
        for (auto &system_port: m_system_ports) {
            if (system_port.get_port_number() == port_number) {
                return true;
            }
        }
        return false;
    }

} // namespace midimagic