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

static const char *midi_message_type_names[] = {
    "Note Off",
    "Note On",
    "Pol Key Press",
    "Control Change",
    "Program Change",
    "Channel Press",
    "Pitch Bend"
};

static const char* midi_msgtype2name(const midi_message::message_type type) {
    return midi_message_type_names[type-0x8];
};

} // namespace midimagic

#endif //MIDIMAGIC_MIDI_TYPES_H
