#ifndef MIDIMAGIC_OUTPUT_H
#define MIDIMAGIC_OUTPUT_H

#include "common.h"
#include <vector>
#include <queue>
#include <memory>

namespace midimagic {
    class midi_message;
    class ad57x4;

    class output_port {
    public:
        explicit output_port(u8 digital_pin, u8 dac_channel, ad57x4 &dac);
        output_port() = delete;
        output_port(const output_port&) = delete;

        bool is_active();
        bool is_note(midi_message &msg);
        void set_note(midi_message &note_on_msg);
        void end_note();
    private:
        u8 m_digital_pin;
        u8 m_dac_channel;
        ad57x4 &m_dac;
        u8 m_current_note;
    };

    class output_demux {
    public:
        enum demux_mode {
            RANDOM,
            OVERWRITE_OLDEST,
            SAME_ON_ALL
        };

        explicit output_demux(demux_mode mode);

        void add_output(std::unique_ptr<output_port> p);
        void add_note(midi_message& msg);
        void remove_note(midi_message& msg);
    private:
        bool set_note(midi_message &msg);
        demux_mode  m_mode;
        std::vector<std::unique_ptr<output_port>> m_ports;
        std::vector<midi_message> m_msgs;
    };
}

#endif //MIDIMAGIC_OUTPUT_H
