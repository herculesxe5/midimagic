#ifndef MIDIMAGIC_SYS_CONFIGS_H
#define MIDIMAGIC_SYS_CONFIGS_H

#include "common.h"
#include "inventory.h"

namespace midimagic {
    struct inventory::output_port_config port0_conf = {
        .port_number = 0
    };

    struct inventory::output_port_config port1_conf = {
        .port_number = 1
    };

    struct inventory::output_port_config port2_conf = {
        .port_number = 2
    };

    struct inventory::output_port_config port3_conf = {
        .port_number = 3
    };

    struct inventory::output_port_config port4_conf = {
        .port_number = 4
    };

    struct inventory::output_port_config port5_conf = {
        .port_number = 5
    };

    struct inventory::output_port_config port6_conf = {
        .port_number = 6
    };

    struct inventory::output_port_config port7_conf = {
        .port_number = 7
    };

    struct inventory::port_group_config default_polyphonic_pg = {
        .id = 0,
        .demux = demux_type::FIFO,
        .midi_channel = 1,
        .input_types = {midi_message::message_type::NOTE_ON, midi_message::message_type::NOTE_OFF},
        .output_port_numbers = {0, 1, 2}
    };

    struct inventory::port_group_config test_pg1 = {
        .id = 1,
        .demux = demux_type::RANDOM,
        .midi_channel = 11,
        .input_types = {midi_message::message_type::POLY_KEY_PRESSURE},
        .output_port_numbers = {7}
    };

    struct inventory::port_group_config test_pg2 = {
        .id = 2,
        .demux = demux_type::IDENTIC,
        .midi_channel = 1,
        .input_types = {midi_message::message_type::PITCH_BEND},
        .output_port_numbers = {3, 4}
    };

    const struct inventory::system_config default_config = (inventory::system_config) {
        .system_ports = {port0_conf, port1_conf, port2_conf, port3_conf, port4_conf, port7_conf},
        .system_port_groups = {default_polyphonic_pg, test_pg1, test_pg2}
    };


} // namespace midimagic
#endif // MIDIMAGIC_SYS_CONFIGS_H