#include <common.h>
#include <SPI.h>
#include <MIDI.h>

#include "ad57x4.h"
#include "midi_types.h"
#include "output.h"

namespace midimagic {
    constexpr u8 power_dacs = PA11;
    constexpr u8 cs_dac0    = PA0;
    constexpr u8 cs_dac1    = PA1;
    constexpr u8 mosi_dac   = PA7;
    constexpr u8 miso_dac   = PA6;
    constexpr u8 clk_dac    = PA5;
    constexpr u8 port1_pin  = PB11;
    constexpr u8 port2_pin  = PB12;
    constexpr u8 port3_pin  = PB13;
    constexpr u8 port4_pin  = PB14;

    SPIClass spi1(mosi_dac, miso_dac, clk_dac);

    ad57x4 dac0(spi1, cs_dac0);
    ad57x4 dac1(spi1, cs_dac1);

    midi::MidiInterface<HardwareSerial> MIDI((HardwareSerial&)Serial1);

    std::unique_ptr<output_port> port0 = std::make_unique<output_port>(
            port1_pin, ad57x4::CHANNEL_A, dac0);
    std::unique_ptr<output_port> port1 = std::make_unique<output_port>(
            port2_pin, ad57x4::CHANNEL_B, dac0);

    output_demux demux0(output_demux::OVERWRITE_OLDEST);
};

void handleNoteOn(byte midi_channel, byte midi_note, byte midi_velo) {
    using namespace midimagic;
    midi_message msg(midi_message::NOTE_ON, midi_channel, midi_note, midi_velo);
    demux0.add_note(msg);
}

void handleNoteOff(byte midi_channel, byte midi_note, byte midi_velo) {
    using namespace midimagic;
    midi_message msg(midi_message::NOTE_OFF, midi_channel, midi_note, midi_velo);
    demux0.remove_note(msg);
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
    demux0.add_output(std::move(port0));
    demux0.add_output(std::move(port1));
}

void loop() {
    using namespace midimagic;
    MIDI.read();
}