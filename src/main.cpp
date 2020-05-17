#include <common.h>
#include <SPI.h>
#include <MIDI.h>

#include "ad57x4.h"
#include "midi_types.h"
#include "output_port.h"

namespace midimagic {
    constexpr u8 power_dacs = PA11;
    constexpr u8 cs_dac0    = PA0;
    constexpr u8 cs_dac1    = PA1;
    constexpr u8 mosi_dac   = PA7;
    constexpr u8 miso_dac   = PA6;
    constexpr u8 clk_dac    = PA5;
    constexpr u8 port1_pin  = PB11;
    constexpr u8 port2_pin  = PB12;

    SPIClass spi1(mosi_dac, miso_dac, clk_dac);

    ad57x4 dac0(spi1, cs_dac0);
    ad57x4 dac1(spi1, cs_dac1);

    midi::MidiInterface<HardwareSerial> MIDI((HardwareSerial&)Serial1);

    output_port port1(port1_pin, ad57x4::ALL_CHANNELS, dac0);
    output_port port2(port2_pin, ad57x4::ALL_CHANNELS, dac1);
};

void handleNoteOn(byte midi_channel, byte midi_note, byte midi_velo) {
    using namespace midimagic;
    midi_message test(midi_message::NOTE_ON, midi_channel, midi_note, midi_velo);
    port1.set_note(test);
    port2.set_note(test);
}

void handleNoteOff(byte midi_channel, byte midi_note, byte midi_velo) {
    using namespace midimagic;
    port1.end_note();
}


void setup() {
    using namespace midimagic;
    // Power up dacs
    pinMode(power_dacs, OUTPUT);
    digitalWrite(power_dacs, HIGH);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    dac0.set_level(0, ad57x4::ALL_CHANNELS);
    dac1.set_level(0, ad57x4::ALL_CHANNELS);
}

void loop() {
    using namespace midimagic;
    MIDI.read();
}