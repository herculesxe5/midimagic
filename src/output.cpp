#include "output.h"
#include "midi_types.h"
#include "ad57x4.h"
#include <cstdlib>

namespace midimagic {
    output_port::output_port(u8 digital_pin, u8 dac_channel, ad57x4 &dac,
                             std::shared_ptr<menu_state> menu, u8 port_number)
        : m_digital_pin(digital_pin)
        , m_dac_channel(dac_channel)
        , m_dac(dac)
        , m_current_note(255)
        , m_menu(menu)
        , m_port_number(port_number)
        , m_menu_action_kind(menu_action::kind::PORT_ACTIVITY) {
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
        //send port activity info to current view
        menu_action a(m_menu_action_kind, menu_action::subkind::PORT_ACTIVE, m_port_number, m_current_note);
        m_menu->notify(a);
    }

    u8 output_port::get_note() {
        return m_current_note;
    }

    void output_port::end_note() {
        m_current_note = 255;
        digitalWrite(m_digital_pin, LOW);
        //send port activity info to current view
        menu_action a(m_menu_action_kind, menu_action::subkind::PORT_NACTIVE, m_port_number);
        m_menu->notify(a);
    }

    const u8 output_port::get_digital_pin() const {
        return m_digital_pin;
    }

    output_demux::output_demux() {
        // nothing to do
    }

    output_demux::~output_demux() {
        // nothing to do
    }

    void output_demux::add_output(output_port p) {
        m_ports.emplace_back(std::make_unique<output_port>(p));
    }

    const std::vector<std::unique_ptr<output_port>>& output_demux::get_output() const {
        return m_ports;
    }

    void output_demux::remove_output(u8 digital_pin) {
        for (auto it = m_ports.begin(); it != m_ports.end(); ) {
            if ((*it)->get_digital_pin() == digital_pin) {
                it = m_ports.erase(it);
                return;
            } else {
                ++it;
            }
        }
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
        return false;
    }

    random_output_demux::random_output_demux() {
        // nothing to do
    }

    random_output_demux::~random_output_demux() {
        // nothing to do
    }

    void random_output_demux::add_note(midi_message &msg) {
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
    }

    identic_output_demux::identic_output_demux() {
        // nothing to do
    }

    identic_output_demux::~identic_output_demux() {
        // nothing to do
    }

    void identic_output_demux::add_note(midi_message &msg) {
        for (auto &port: m_ports) {
            port->set_note(msg);
        }
    }

    fifo_output_demux::fifo_output_demux() {
        // nothing to do
    }

    fifo_output_demux::~fifo_output_demux() {
        // nothing to do
    }

    void fifo_output_demux::add_note(midi_message &msg) {
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
    }
} // namespace midimagic
