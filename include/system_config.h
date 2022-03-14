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