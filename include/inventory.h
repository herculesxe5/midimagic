#ifndef MIDIMAGIC_INVENTORY_H
#define MIDIMAGIC_INVENTORY_H

#include "common.h"
#include "port_group.h"
#include "output.h"
#include "ad57x4.h"
#include "midi_types.h"

namespace midimagic {
    class inventory {
    public:
        const u8 port_number2digital_pin[8] {
            PB9, // port0
            PB8, // port1
            PB7, // port2
            PB6, // port3
            PB5, // port4
            PB4, // port5
            PB3, // port6
            PA15 // port7
        };
        struct output_port_config {
            u8 port_number;
        };

        struct port_group_config {
            u8 id;
            demux_type demux;
            u8 midi_channel;
            std::vector<midi_message::message_type> input_types;
            std::vector<u8> output_port_numbers;
        };

        struct system_config {
            std::vector<struct output_port_config> system_ports;
            std::vector<struct port_group_config> system_port_groups;
        };

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

        void submit_portgroup_change(const u8 system_pg_id); // inform inventory about port group config change, updates system_config
        void submit_portgroup_delete(const u8 system_pg_id); // inform inventory about deleted port group, updates system config
        void submit_portgroup_add(const u8 system_pg_id); // inform inventory about new port group, updates system config
        std::shared_ptr<output_port> get_output_port(const u8 port_number); // returns pointer to output_port by port number, creates object if needed
        std::shared_ptr<group_dispatcher> get_group_dispatcher(); // returns pointer to the system port group dispatcher
        void apply_config(const struct system_config& new_config); // setup system as in new_config
    private:
        std::shared_ptr<group_dispatcher> m_group_dispatcher;
        std::shared_ptr<menu_action_queue> m_menu_q;
        ad57x4 &m_dac0, &m_dac1;
        struct system_config m_system_config;
        std::vector<std::shared_ptr<output_port>> m_system_ports;

        void flush(); // destroys all port groups and deletes from system_config

        // vvv these functions alter the system setup vvv
        // creates output_port in m_system_ports
        void spawn_port(const u8 config_port_number);
        // create new port group as per id in system_config, returns new id
        const u8 spawn_port_group(std::vector<struct port_group_config>::iterator config_pg_it);

        // vvv these functions update system_config vvv
        // update system_config with new properties
        void rescan_port_group(const std::vector<std::unique_ptr<port_group>>::const_iterator system_pg_it);
        // adds new port group to system_config
        void add_port_group(const std::vector<std::unique_ptr<port_group>>::const_iterator system_pg_it);
        // deletes port group from system_config
        void forget_port_group(const std::vector<std::unique_ptr<port_group>>::const_iterator system_pg_it);

        std::vector<struct port_group_config>::iterator config_pg_it_by_id(u8 config_pg_id);
        const std::vector<std::unique_ptr<port_group>>::const_iterator system_pg_it_by_id(const u8 system_pg_id) const;
        const bool port_exists(const u8 port_number) const;
    };
} // namespace midimagic
#endif // MIDIMAGIC_INVENTORY_H