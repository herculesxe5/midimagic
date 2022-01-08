#ifndef MIDIMAGIC_SYSTEM_CONFIG_H
#define MIDIMAGIC_SYSTEM_CONFIG_H

#include "common.h"
#include "output.h"
#include "midi_types.h"

namespace midimagic {

    struct output_port_config {
        u8 port_number;
        u8 clock_rate;
        bool velocity_output;
    };

    struct port_group_config {
        u8 id;
        demux_type demux;
        u8 midi_channel;
        u8 cont_controller_number;
        i8 transpose_offset;
        std::vector<midi_message::message_type> input_types;
        std::vector<u8> output_port_numbers;
    };

    struct system_config {
        std::vector<struct output_port_config> system_ports;
        std::vector<struct port_group_config> system_port_groups;
    };
} // namespace midimagic

#endif // MIDIMAGIC_SYSTEM_CONFIG_H