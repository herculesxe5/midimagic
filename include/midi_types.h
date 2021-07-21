#ifndef MIDIMAGIC_MIDI_TYPES_H
#define MIDIMAGIC_MIDI_TYPES_H

#include "common.h"

namespace midimagic {

struct midi_message {
    enum message_type {
        NOTE_OFF = 0x8,
        NOTE_ON,
        POLY_KEY_PRESSURE,
        CONTROL_CHANGE,
        PROGRAM_CHANGE,
        CHANNEL_PRESSURE,
        PITCH_BEND
    };
    message_type type;
    u8 channel;
    u8 data0;
    u8 data1;

    midi_message(message_type type, u8 channel, u8 data0, u8 data1) :
        type(type),
        channel(channel),
        data0(data0),
        data1(data1) {
    };

    bool is_same_note(midi_message &m) {
        if(data0 == m.data0)
            return true;
        else
            return false;
    };
};

} // namespace midimagic

#endif //MIDIMAGIC_MIDI_TYPES_H
