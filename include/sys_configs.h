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

#ifndef MIDIMAGIC_SYS_CONFIGS_H
#define MIDIMAGIC_SYS_CONFIGS_H

#include "common.h"
#include "system_config.h"

namespace midimagic {
    struct output_port_config port0_conf = {
        .port_number = 0,
        .clock_rate = 24,
        .velocity_output = false
    };

    struct output_port_config port1_conf = {
        .port_number = 1,
        .clock_rate = 24,
        .velocity_output = false
    };

    struct output_port_config port2_conf = {
        .port_number = 2,
        .clock_rate = 24,
        .velocity_output = false
    };

    struct output_port_config port3_conf = {
        .port_number = 3,
        .clock_rate = 24,
        .velocity_output = false
    };

    struct output_port_config port4_conf = {
        .port_number = 4,
        .clock_rate = 24,
        .velocity_output = false
    };

    struct output_port_config port5_conf = {
        .port_number = 5,
        .clock_rate = 24,
        .velocity_output = false
    };

    struct output_port_config port6_conf = {
        .port_number = 6,
        .clock_rate = 24,
        .velocity_output = false
    };

    struct output_port_config port7_conf = {
        .port_number = 7,
        .clock_rate = 24,
        .velocity_output = false
    };

    struct port_group_config default_polyphonic_pg = {
        .id = 0,
        .demux = demux_type::IDENTIC,
        .midi_channel = 1,
        .cont_controller_number = 0,
        .transpose_offset = 0,
        .input_types = {midi_message::message_type::NOTE_ON, midi_message::message_type::NOTE_OFF, midi_message::message_type::PITCH_BEND},
        .output_port_numbers = {0, 1, 2}
    };

    struct port_group_config test_pg1 = {
        .id = 1,
        .demux = demux_type::IDENTIC,
        .midi_channel = 1,
        .cont_controller_number = 0,
        .transpose_offset = 0,
        .input_types = {midi_message::message_type::CLOCK},
        .output_port_numbers = {7}
    };

    struct port_group_config test_pg2 = {
        .id = 2,
        .demux = demux_type::IDENTIC,
        .midi_channel = 1,
        .cont_controller_number = 0,
        .transpose_offset = 0,
        .input_types = {midi_message::message_type::PITCH_BEND},
        .output_port_numbers = {3}
    };

    struct port_group_config test_cc = {
        .id = 3,
        .demux = demux_type::IDENTIC,
        .midi_channel = 1,
        .cont_controller_number = 1,
        .transpose_offset = 0,
        .input_types = {midi_message::message_type::CONTROL_CHANGE},
        .output_port_numbers = {4}
    };

    const struct system_config default_config = (system_config) {
        .system_ports = {port0_conf, port1_conf, port2_conf, port3_conf, port4_conf, port7_conf},
        .system_port_groups = {default_polyphonic_pg, test_pg1, test_pg2, test_cc}
    };


} // namespace midimagic
#endif // MIDIMAGIC_SYS_CONFIGS_H