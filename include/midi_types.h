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
        PITCH_BEND,
        CLOCK = 0xf8,
        START = 0xfa,
        CONTINUE,
        STOP
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
    "Pol Key Pr",
    "Contr Chg",
    "Progr Chg",
    "Chan Press",
    "Pitch Bend",
    "Clock"
};

static const char *midi_message_type_long_names[] = {
    "Note Off",
    "Note On",
    "Polyphonic Key Pres",
    "Control Change",
    "Program Change",
    "Channel Pressure",
    "Pitch Bend",
    "Timing Clock"
};

static const char* midi_msgtype2name(const midi_message::message_type type) {
    if (type < 0xf) {
        return midi_message_type_names[type - 0x8];
    } else if (type == midi_message::message_type::CLOCK) {
        return midi_message_type_names[7];
    } else {
        return "unknown type";
    }
};

} // namespace midimagic

#endif //MIDIMAGIC_MIDI_TYPES_H
