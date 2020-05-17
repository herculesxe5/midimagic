#ifndef MIDIMAGIC_OUTPUT_PORT_H
#define MIDIMAGIC_OUTPUT_PORT_H

#include "common.h"

namespace midimagic {
    class midi_message;
    class ad57x4;

    class output_port {
    public:
        explicit output_port(u8 digital_pin, u8 dac_channel, ad57x4 &dac);
        output_port() = delete;
        output_port(const output_port&) = delete;

        void set_note(midi_message &note_on_msg);
        void end_note();
    private:
        u8 m_digital_pin;
        u8 m_dac_channel;
        ad57x4 &m_dac;
    };
}

#endif //MIDIMAGIC_OUTPUT_PORT_H
