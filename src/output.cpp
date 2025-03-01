/******************************************************************************
 *                                                                            *
 * Copyright 2021-2024 Lukas Jünger and Adrian Krause                         *
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

    output_port::clock_mode& operator++(output_port::clock_mode& cm) {
        return cm = (cm == output_port::clock_mode::SIGNAL_TRIGGER_STOP) ? output_port::clock_mode::SYNC : static_cast<output_port::clock_mode>(static_cast<int>(cm) + 1);
    }

    output_port::clock_mode& operator--(output_port::clock_mode& cm) {
        return cm = (cm == output_port::clock_mode::SYNC) ? output_port::clock_mode::SIGNAL_TRIGGER_STOP : static_cast<output_port::clock_mode>(static_cast<int>(cm) - 1);
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
        , m_clock_mode(clock_mode::SYNC)
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
                    //FIXME adjust voltage scaling
                    steps = msg.data1 << 3;
                }
                break;
            case midi_message::message_type::POLY_KEY_PRESSURE :
                // output unipolar representation of the value
                steps = msg.data1 << 3;
                inhibit_digital_pin = true;
                break;
            case midi_message::message_type::CONTROL_CHANGE :
                // reassemble 14 bit value
                cc_value = (msg.data0 << 7) + msg.data1;
                steps = cc_value << 1;
                // model switch functionality as in MIDI spec
                if (cc_value < 64) {
                    digital_pin_control = LOW;
                }
                port_status = menu_action::subkind::PORT_ACTIVE_CC;
                break;
            case midi_message::message_type::CHANNEL_PRESSURE :
                // output unipolar representation of the value
                steps = msg.data0 << 3;
                inhibit_digital_pin = true;
                break;
            case midi_message::message_type::PITCH_BEND :
                inhibit_digital_pin = true;
                // reassemble signed integer
                PB_value = (i16) (msg.data0 << 8);
                PB_value = PB_value | msg.data1;
                // calculate offset in halftone steps
                PB_offset = (static_cast<float>(272) / 8192) * PB_value;
                // add offset to the current note if not cleared
                if (m_current_note != 255) {
                    delta = m_current_note - 60;
                    steps = (delta * 136 + PB_offset) << 2;
                } else {
                    // output raw value
                    steps = PB_offset << 2;
                }
                break;
            case midi_message::message_type::STOP :
                // If we are set to trigger mode with 'stop' turn port "on".
                // Turn port "off" for all other modes
                // Break flow everytime
                inhibit_dac_update = true;
                if (m_clock_mode == clock_mode::SIGNAL_TRIGGER_STOP) {
                    port_status = menu_action::subkind::PORT_ACTIVE_CLK;
                } else {
                    port_status = menu_action::subkind::PORT_NACTIVE;
                    digital_pin_control = LOW;
                }
                break;
            case midi_message::message_type::START :
                // If we get START messsage in TRIGGER_START mode turn port "on" and break execution.
                // If mode is not SYNC or GATE turn port "off" and break execution.
                // In SYNC or GATE mode continue to next case.
                inhibit_dac_update = true;
                if (m_clock_mode == clock_mode::SIGNAL_TRIGGER_START) {
                    port_status = menu_action::subkind::PORT_ACTIVE_CLK;
                    break;
                } else if (m_clock_mode > clock_mode::SIGNAL_GATE) {
                    port_status = menu_action::subkind::PORT_NACTIVE;
                    digital_pin_control = LOW;
                    break;
                }
                // continue to next case for other clock modes
            case midi_message::message_type::CONTINUE :
                // Here we can stumble upon all CONTINUE messages and if in GATE mode START messages will arrive here too.
                // In SYNC mode reset the port state for either message.
                // In GATE or continue-trigger mode turn port "on".
                // Turn port off for all other modes (i.e. the other trigger modes).
                inhibit_dac_update = true;
                if (m_clock_mode == clock_mode::SYNC) {
                    // reset clock_count on receipt of START or CONTINUE to resync on the next clock
                    m_clock_count = 0;
                    digital_pin_control = LOW;
                    port_status = menu_action::subkind::PORT_NACTIVE;
                } else if ((m_clock_mode == clock_mode::SIGNAL_GATE) || (m_clock_mode == clock_mode::SIGNAL_TRIGGER_CONT)) {
                    port_status = menu_action::subkind::PORT_ACTIVE_CLK;
                } else {
                    port_status = menu_action::subkind::PORT_NACTIVE;
                    digital_pin_control = LOW;
                }
                break;
            case midi_message::message_type::CLOCK :
                // raise digital pin on clock_count 1,
                // lower pin after duty cycle of 4 clocks,
                // reset clock_count when at value of clock_rate
                inhibit_dac_update = true;
                if (m_clock_mode != clock_mode::SYNC) {
                    inhibit_digital_pin = true;
                    inhibit_menu_action = true;
                    break;
                }
                port_status = menu_action::subkind::PORT_ACTIVE_CLK;
                m_clock_count++;
                if (m_clock_count >= m_clock_rate) {
                    m_clock_count = 0;
                } else if (m_clock_count == 5) {
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

    void output_port::set_clock_rate(const u8 clr) {
        m_clock_rate = clr;
    }

    const u8 output_port::get_clock_rate() const {
        return m_clock_rate;
    }

    void output_port::reset_clock() {
        midi_message msg(midi_message::message_type::START, 0, 0, 0);
        set_note(msg);
    }

    void output_port::set_velocity_switch() {
        if (m_output_velocity) {
            m_output_velocity = false;
        } else {
            m_output_velocity = true;
        }
    }

    const bool output_port::get_velocity_switch() const {
        return m_output_velocity;
    }

    void output_port::set_clock_mode(const clock_mode cm) {
        m_clock_mode = cm;
    }

    const output_port::clock_mode output_port::get_clock_mode() const {
        return m_clock_mode;
    }

    output_demux::output_demux(const demux_type type)
        : m_type(type) {
        // nothing to do
    }

    output_demux::~output_demux() {
        // nothing to do
    }

    void output_demux::add_output(std::shared_ptr<output_port> p) {
        for (auto& port: m_ports) {
            if (port->get_port_number() == p->get_port_number()) {
                return;
            }
        }
        m_ports.push_back(std::move(p));
    }

    const std::vector<std::shared_ptr<output_port>>& output_demux::get_output() const {
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
