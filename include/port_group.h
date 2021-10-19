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
    private:
        std::vector<std::unique_ptr<port_group>> m_port_groups;
        u8 m_last_group_id;

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

        void send_input(midi_message& m);
    private:
        midi_message parse_cc(midi_message& m);

        std::unique_ptr<output_demux> m_demux;
        std::vector<midi_message::message_type> m_input_types;
        u8 m_input_channel;
        u8 m_cc_number;
        u8 m_cc_MSB_value;
        const u8 k_id;
    };
} // namespace midimagic
#endif // MIDIMAGIC_PORT_GROUP_H