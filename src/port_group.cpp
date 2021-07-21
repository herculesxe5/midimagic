#include "port_group.h"

namespace midimagic {
    group_dispatcher::group_dispatcher()
        : m_last_group_id(0) {
        // nothing to do
    }

    group_dispatcher::~group_dispatcher() {
        // nothing to do
    }

    void group_dispatcher::add_port_group(const demux_type dt,
                        const midi_message::message_type msg_type, const u8 channel) {
        m_port_groups.emplace_back(
            std::make_unique<port_group>(get_next_id(), dt, msg_type, channel));
    }

    void group_dispatcher::remove_port_group(const u8 id) {
        for (auto it = m_port_groups.begin(); it != m_port_groups.end(); ) {
            if ((*it)->get_id() == id) {
                it = m_port_groups.erase(it);
                return;
            } else {
                ++it;
            }
        }
    }

    const std::vector<std::unique_ptr<port_group>>& group_dispatcher::get_port_groups() const {
        return m_port_groups;
    }

    void group_dispatcher::add_message(midi_message& m) {
        sieve(m);
    }

    void group_dispatcher::sieve(midi_message& m) {
        for (auto &port_group: m_port_groups) {
            if (m.channel == port_group->get_midi_channel() &&
                port_group->has_msg_type(m.type)) {
                port_group->send_input(m);
            }
        }
    }

    const u8 group_dispatcher::get_next_id() {
        return ++m_last_group_id;
    }

    port_group::port_group(const u8 id, const demux_type dt,
                           const midi_message::message_type msg_type, const u8 channel)
        : m_id(id)
        , m_input_channel(channel) {
        m_input_types.push_back(msg_type);
        set_demux(dt);
    }

    port_group::~port_group() {
        // nothing to do
    }

    void port_group::set_demux(const demux_type type) {
        switch (type) {
            case demux_type::FIFO:
                m_demux = std::make_unique<fifo_output_demux>();
                break;
            case demux_type::IDENTIC:
                m_demux = std::make_unique<identic_output_demux>();
                break;
            case demux_type::RANDOM:
                m_demux = std::make_unique<random_output_demux>();
                break;
            default:
                __builtin_trap();
                break;
        }
    }

    const output_demux& port_group::get_demux() const {
        return *m_demux;
    }

    void port_group::add_midi_input(const midi_message input_type) {
        m_input_types.push_back(input_type.type);
        m_input_channel = input_type.channel;
    }

    void port_group::remove_msg_type(const midi_message input_type) {
        for (auto it = m_input_types.begin(); it != m_input_types.end(); ) {
            if (*it == input_type.type) {
                it = m_input_types.erase(it);
            } else {
                ++it;
            }
        }
    }

    const std::vector<midi_message::message_type>& port_group::get_msg_types() const {
        return m_input_types;
    }

    const bool port_group::has_msg_type(const midi_message::message_type msg_type) const {
        for (auto it = m_input_types.begin(); it != m_input_types.end(); ) {
            if (*it == msg_type) {
                return true;
            } else {
                ++it;
            }
        }
        return false;
    }

    const u8 port_group::get_midi_channel() const {
        return m_input_channel;
    }

    void port_group::add_port(output_port port) {
        m_demux->add_output(port);
    }

    void port_group::remove_port(u8 digital_pin) {
        m_demux->remove_output(digital_pin);
    }

    const u8 port_group::get_id() const {
        return m_id;
    }

    void port_group::send_input(midi_message& m) {
        m_demux->add_note(m);
    }
} // namespace midimagic