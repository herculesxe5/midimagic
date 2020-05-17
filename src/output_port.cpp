#include "output_port.h"
#include "midi_types.h"
#include "ad57x4.h"

namespace midimagic {
    output_port::output_port(u8 digital_pin, u8 dac_channel, ad57x4 &dac) :
        m_digital_pin(digital_pin),
        m_dac_channel(dac_channel),
        m_dac(dac){
        pinMode(digital_pin, OUTPUT);
    }

    void output_port::set_note(midi_message &msg) {
        // assume c1 tuning
        i8 delta = msg.data0 - 60;
        // calculate dac level
        i16 steps = delta * 136 << 2;
        m_dac.set_level(steps, m_dac_channel);
        digitalWrite(m_digital_pin, HIGH);
    }

    void output_port::end_note() {
        digitalWrite(m_digital_pin, LOW);
    }
} // namespace midimagic
