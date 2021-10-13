#include "port_group.h"

namespace midimagic {
    group_dispatcher::group_dispatcher()
        : m_last_group_id(0) {
        // nothing to do
    }

    group_dispatcher::~group_dispatcher() {
        // nothing to do
    }

    void group_dispatcher::add_port_group(const demux_type dt, const u8 channel) {
        m_port_groups.emplace_back(
            std::make_unique<port_group>(get_next_id(), dt, channel));
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
        // catch program change messages, as these are supposed to control the device
        if (m.type == midi_message::message_type::PROGRAM_CHANGE) {
            return;
        }
        sieve(m);
    }

    void group_dispatcher::sieve(midi_message& m) {
        // system common and real time messages are channel independent and to be send to all receivers
        if (m.type > 0xf0) {
            for (auto &port_group: m_port_groups) {
                switch (m.type) {
                    case midi_message::message_type::START :
                        // just slide through
                    case midi_message::message_type::CONTINUE :
                        // just slide through
                    case midi_message::message_type::CLOCK :
                        if (port_group->has_msg_type(midi_message::message_type::CLOCK)) {
                            port_group->send_input(m);
                        }
                        break;
                    default :
                        // nothing to do
                        break;
                }
            }
        } else {
            for (auto &port_group: m_port_groups) {
                if (m.channel == port_group->get_midi_channel()
                    && port_group->has_msg_type(m.type)) {
                    port_group->send_input(m);
                }
            }
        }
    }

    const u8 group_dispatcher::get_next_id() {
        return ++m_last_group_id;
    }

    port_group::port_group(const u8 id, const demux_type dt, const u8 channel)
        : k_id(id)
        , m_input_channel(channel)
        , m_cc_number(0)
        , m_cc_MSB_value(0) {
        set_demux(dt);
    }

    port_group::~port_group() {
        // nothing to do
    }

    void port_group::set_demux(const demux_type type) {
        std::unique_ptr<output_demux> new_demux;
        switch (type) {
            case demux_type::FIFO:
                new_demux = std::make_unique<fifo_output_demux>(type);
                break;
            case demux_type::IDENTIC:
                new_demux = std::make_unique<identic_output_demux>(type);
                break;
            case demux_type::RANDOM:
                new_demux = std::make_unique<random_output_demux>(type);
                break;
            default:
                __builtin_trap();
                break;
        }
        if (m_demux) {
            for (auto& port: m_demux->get_output()) {
                new_demux->add_output(std::move(*port));
            }
        }
        m_demux = std::move(new_demux);
    }

    void port_group::set_midi_channel(const u8 ch) {
        m_input_channel = ch;
    }

    const output_demux& port_group::get_demux() const {
        return *m_demux;
    }

    void port_group::add_midi_input(const midi_message::message_type input_type) {
        if (!has_msg_type(input_type)) {
            m_input_types.push_back(input_type);
        }
    }

    void port_group::remove_msg_type(const midi_message::message_type input_type) {
        for (auto it = m_input_types.begin(); it != m_input_types.end(); ) {
            if (*it == input_type) {
                it = m_input_types.erase(it);
                return;
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

    void port_group::remove_port(u8 port_number) {
        m_demux->remove_output(port_number);
    }

    const u8 port_group::get_id() const {
        return k_id;
    }

    void port_group::send_input(midi_message& m) {
        if (m.type == midi_message::message_type::NOTE_OFF) {
            m_demux->remove_note(m);
        } else if (m.type == midi_message::message_type::CONTROL_CHANGE) {
            if (m_cc_number == m.data0 || m_cc_number == m.data0 - 32) {
                auto cc_msg = parse_cc(m);
                m_demux->add_note(cc_msg);
            }
        } else {
            m_demux->add_note(m);
        }
    }

    midi_message port_group::parse_cc(midi_message& m) {
        // parse controller value and return midi_message with format:
        // type::CONTROL_CHANGE, channel, value MSB, value LSB
        u8 cc_LSB_value = 0;
        if (m_cc_number == m.data0) {
            m_cc_MSB_value = m.data1;
        } else if ((m_cc_number == m.data0 - 32) && (m.data0 < 64)) {
            cc_LSB_value = m.data1;
        }
        midi_message out_msg(m.type, m.channel, m_cc_MSB_value, cc_LSB_value);
        return out_msg;
    }
} // namespace midimagic