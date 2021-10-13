#include "output.h"
#include "midi_types.h"
#include "ad57x4.h"
#include "menu.h"
#include <cstdlib>

namespace midimagic {
    const char *demux_type_names[] = {
        "Random",
        "Identic",
        "FIFO"
    };

    demux_type& operator++(demux_type& dt) {
        return dt = (dt == demux_type::FIFO) ? demux_type::RANDOM : static_cast<demux_type>(static_cast<int>(dt)+1);
    }

    demux_type& operator--(demux_type& dt) {
        return dt = (dt == demux_type::RANDOM) ? demux_type::FIFO : static_cast<demux_type>(static_cast<int>(dt)-1);
    }

    const char* demux_type2name(demux_type type) {
        return demux_type_names[type];
    }

    output_port::output_port(u8 digital_pin, u8 dac_channel, ad57x4 &dac,
                             std::shared_ptr<menu_action_queue> menu, u8 port_number)
        : m_digital_pin(digital_pin)
        , m_dac_channel(dac_channel)
        , m_dac(dac)
        , m_current_note(255)
        , m_clock_count(0)
        , m_clock_rate(24)
        , m_output_velocity(false)
        , m_menu(menu)
        , m_port_number(port_number) {
        pinMode(m_digital_pin, OUTPUT);
    }

    bool output_port::is_active() {
        return digitalRead(m_digital_pin);
    }

    bool output_port::is_note(midi_message &msg) {
        return m_current_note == msg.data0;
    }

    void output_port::set_note(midi_message &msg) {
        i8 delta = 0;
        i16 steps = 0, PB_value, PB_offset;
        u8 digital_pin_control = HIGH;
        u16 cc_value;
        bool inhibit_dac_update = false, inhibit_digital_pin = false, inhibit_menu_action = false;
        menu_action::subkind port_status = menu_action::subkind::PORT_ACTIVE;
        switch (msg.type) {
            case midi_message::message_type::NOTE_ON :
                m_current_note = msg.data0;
                if (!m_output_velocity) {
                    // assume c1 tuning
                    delta = msg.data0 - 60;
                    // calculate dac level
                    steps = delta * 136 << 2;
                } else {
                    steps = msg.data1 << 2;
                }
                break;
            case midi_message::message_type::POLY_KEY_PRESSURE :
                // output unipolar representation of the value
                steps = msg.data1 << 2;
                inhibit_digital_pin = true;
                break;
            case midi_message::message_type::CONTROL_CHANGE :
                // reassemble 14 bit value
                cc_value = (msg.data0 << 7) + msg.data1;
                steps = cc_value << 2;
                // model switch functionality as in MIDI spec
                if (cc_value < 64) {
                    digital_pin_control = LOW;
                }
                break;
            case midi_message::message_type::CHANNEL_PRESSURE :
                // output unipolar representation of the value
                steps = msg.data0 << 2;
                inhibit_digital_pin = true;
                break;
            case midi_message::message_type::PITCH_BEND :
                inhibit_digital_pin = true;
                //FIXME test this
                // reassemble signed integer
                PB_value = msg.data0 << 8;
                PB_value = PB_value + msg.data1;
                // calculate offset in halftone steps
                PB_offset = (272 / 8192) * PB_value;
                // add offset to the current note if not cleared
                if (m_current_note != 255) {
                    delta = m_current_note - 60;
                    steps = (delta * 136 + PB_offset) << 2;
                } else {
                    // output raw value
                    steps = PB_offset << 2;
                }
                break;
            case midi_message::message_type::START :
                // just slide through
            case midi_message::message_type::CONTINUE :
                // reset clock_count on receipt of START or CONTINUE to resync on the next clock
                m_clock_count = 0;
                inhibit_dac_update = true;
                digital_pin_control = LOW;
                port_status = menu_action::subkind::PORT_NACTIVE;
                break;
            case midi_message::message_type::CLOCK :
                // raise digital pin on clock_count 1,
                // lower pin after duty cycle of 5 clocks,
                // reset clock_count when at value of clock_rate
                inhibit_dac_update = true;
                m_clock_count++;
                if (m_clock_count >= m_clock_rate) {
                    m_clock_count = 0;
                } else if (m_clock_count == 6) {
                    digital_pin_control = LOW;
                    port_status = menu_action::subkind::PORT_NACTIVE;
                } else if (m_clock_count == 1) {
                    // nothing to do, use default signaling
                } else {
                    inhibit_digital_pin = true;
                    inhibit_menu_action = true;
                }
                break;
            default :
                // nothing to do
                break;
        }
        if (!inhibit_dac_update) {
            m_dac.set_level(steps, m_dac_channel);
        }
        if (!inhibit_digital_pin) {
        digitalWrite(m_digital_pin, digital_pin_control);
        }
        if (!inhibit_menu_action) {
            // send port activity info to current view
            menu_action a(menu_action::kind::PORT_ACTIVITY, port_status, m_port_number, m_current_note);
            m_menu->add_menu_action(a);
        }
    }

    const u8 output_port::get_note() const {
        return m_current_note;
    }

    void output_port::end_note() {
        m_current_note = 255;
        digitalWrite(m_digital_pin, LOW);
        // send port activity info to current view
        menu_action a(menu_action::kind::PORT_ACTIVITY, menu_action::subkind::PORT_NACTIVE, m_port_number);
        m_menu->add_menu_action(a);
    }

    const u8 output_port::get_digital_pin() const {
        return m_digital_pin;
    }

    const u8 output_port::get_port_number() const {
        return m_port_number;
    }

    output_demux::output_demux(const demux_type type)
        : m_type(type) {
        // nothing to do
    }

    output_demux::~output_demux() {
        // nothing to do
    }

    void output_demux::add_output(output_port p) {
        for (auto& port: m_ports) {
            if (port->get_port_number() == p.get_port_number()) {
                return;
            }
        }
        m_ports.emplace_back(std::make_unique<output_port>(p));
    }

    const std::vector<std::unique_ptr<output_port>>& output_demux::get_output() const {
        return m_ports;
    }

    const demux_type output_demux::get_type() const {
        return m_type;
    }

    void output_demux::remove_output(u8 port_number) {
        for (auto it = m_ports.begin(); it != m_ports.end(); ) {
            if ((*it)->get_port_number() == port_number) {
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

    random_output_demux::random_output_demux(const demux_type type)
        : output_demux(type) {
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

    identic_output_demux::identic_output_demux(const demux_type type)
        : output_demux(type) {
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

    fifo_output_demux::fifo_output_demux(const demux_type type)
        : output_demux(type) {
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
