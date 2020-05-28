#include "output.h"
#include "midi_types.h"
#include "ad57x4.h"

#include <cstdlib>

namespace midimagic {
    output_port::output_port(u8 digital_pin, u8 dac_channel, ad57x4 &dac) :
        m_digital_pin(digital_pin),
        m_dac_channel(dac_channel),
        m_dac(dac),
        m_current_note(255) {
        pinMode(m_digital_pin, OUTPUT);
    }

    bool output_port::is_active() {
        return digitalRead(m_digital_pin);
    }

    bool output_port::is_note(midi_message &msg) {
        return m_current_note == msg.data0;
    }

    void output_port::set_note(midi_message &msg) {
        m_current_note = msg.data0;
        // assume c1 tuning
        i8 delta = msg.data0 - 60;
        // calculate dac level
        i16 steps = delta * 136 << 2;
        m_dac.set_level(steps, m_dac_channel);
        digitalWrite(m_digital_pin, HIGH);
    }

    void output_port::end_note() {
        m_current_note = 255;
        digitalWrite(m_digital_pin, LOW);
    }

    output_demux::output_demux(demux_mode mode) :
        m_mode(mode),
        m_ports(),
        m_msgs() {
        // nothing to do
    }

    void output_demux::add_output(std::unique_ptr<output_port> p) {
        m_ports.emplace_back(std::move(p));
    }

    void output_demux::add_note(midi_message &msg) {
        if (m_mode == OVERWRITE_OLDEST) {
            if (!set_note(msg)) {
                midi_message tmp = m_msgs.front();
                for (auto &port: m_ports) {
                    if (port->is_note(tmp)) {
                        port->set_note(msg);
                        m_msgs.push_back(msg);
                    }
                }
                for(auto it = m_msgs.begin(); it != m_msgs.end();) {
                    if((*it).is_same_note(tmp))
                        it = m_msgs.erase(it);
                    else
                        ++it;
                }
            }
        } else if (m_mode == RANDOM) {
            if(!set_note(msg)) {
                int rand = std::rand() % m_msgs.size();
                midi_message tmp = m_msgs[rand];
                for (auto &port: m_ports) {
                    if (port->is_note(tmp)) {
                        port->set_note(msg);
                        m_msgs.push_back(msg);
                    }
                }
                for(auto it = m_msgs.begin(); it != m_msgs.end(); ) {
                    if((*it).is_same_note(tmp))
                        it = m_msgs.erase(it);
                    else
                        ++it;
                }
            }
        } else if (m_mode == SAME_ON_ALL) {
            for (auto &port: m_ports) {
                port->set_note(msg);
            }
        }
        return;
    }

    void output_demux::remove_note(midi_message &msg) {
        for(auto &port: m_ports) {
            if (port->is_note(msg)) {
                port->end_note();
            }
        }
        for(auto it = m_msgs.begin(); it != m_msgs.end();) {
            if((*it).is_same_note(msg))
                it = m_msgs.erase(it);
            else
                ++it;
        }
    }

    bool output_demux::set_note(midi_message &msg) {
        if (m_msgs.size() == m_ports.size())
            return false;
        for (auto &port: m_ports) {
            if (!port->is_active()) {
                port->set_note(msg);
                m_msgs.push_back(msg);
                return true;
            }
        }
    }
} // namespace midimagic
