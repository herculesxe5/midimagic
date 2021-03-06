/******************************************************************************
 *                                                                            *
 * Copyright 2021 Adrian Krause                                               *
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

#ifndef MIDIMAGIC_PORT_GROUP_H
#define MIDIMAGIC_PORT_GROUP_H

#include <vector>
#include "output.h"
#include "midi_types.h"

namespace midimagic {

    class port_group;

    class group_dispatcher {
    public:
        group_dispatcher();
        group_dispatcher(const group_dispatcher&) = delete;
        ~group_dispatcher();

        void add_port_group(const demux_type dt, const u8 channel);
        void remove_port_group(const u8 id);
        const std::vector<std::unique_ptr<port_group>>& get_port_groups() const;

        void add_message(midi_message& m);
        void activate_capture_mode();
        const bool got_capture() const;
        const midi_message get_capture() const;
    private:
        std::vector<std::unique_ptr<port_group>> m_port_groups;
        u8 m_last_group_id;
        bool m_capture_mode, m_capture_ready;
        midi_message m_captured_message;

        void sieve(midi_message& m);
        const u8 get_next_id();
    };

    class port_group {
    public:
        explicit port_group(const u8 id, const demux_type dt, const u8 channel);
        port_group(port_group&&) = default;
        port_group() = delete;
        port_group(const port_group&) = delete;
        ~port_group();

        void set_demux(const demux_type type);
        const output_demux& get_demux() const;
        void set_midi_channel(const u8 ch);
        const u8 get_midi_channel() const;
        void add_midi_input(const midi_message::message_type input_type);
        void remove_msg_type(const midi_message::message_type input_type);
        const std::vector<midi_message::message_type>& get_msg_types() const;
        const bool has_msg_type(const midi_message::message_type msg_type) const;

        void add_port(std::shared_ptr<output_port> port);
        void remove_port(u8 port_number);
        const u8 get_id() const;
        void set_cc(const u8 cc_number);
        const u8 get_cc() const;
        void set_transpose(const i8 transpose_offset);
        const i8 get_transpose() const;

        void send_input(midi_message& m);
    private:
        midi_message parse_cc(midi_message& m);

        std::unique_ptr<output_demux> m_demux;
        std::vector<midi_message::message_type> m_input_types;
        u8 m_input_channel;
        u8 m_cc_number;
        u8 m_cc_MSB_value;
        i8 m_transpose_offset;
        const u8 k_id;
    };
} // namespace midimagic
#endif // MIDIMAGIC_PORT_GROUP_H