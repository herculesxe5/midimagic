#include <common.h>
#include <SPI.h>
#include <MIDI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

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
    constexpr u8 rot_sw     = PB7;
    constexpr u8 rot_clk    = PB8;
    constexpr u8 rot_dt     = PB9;
    constexpr u8 disp_scl   = PB10;
    constexpr u8 disp_sca   = PB11;
    constexpr u8 port0_pin  = PB12;
    constexpr u8 port1_pin  = PB13;
    constexpr u8 port2_pin  = PB14;
    constexpr u8 port3_pin  = PB15;



    SPIClass spi1(mosi_dac, miso_dac, clk_dac);

    ad57x4 dac0(spi1, cs_dac0);
    ad57x4 dac1(spi1, cs_dac1);

    midi::MidiInterface<HardwareSerial> MIDI((HardwareSerial&)Serial1);

    std::unique_ptr<output_port> port0 = std::make_unique<output_port>(
            port0_pin, ad57x4::CHANNEL_A, dac1);
    std::unique_ptr<output_port> port1 = std::make_unique<output_port>(
            port1_pin, ad57x4::CHANNEL_B, dac1);
    std::unique_ptr<output_port> port2 = std::make_unique<output_port>(
            port2_pin, ad57x4::CHANNEL_C, dac1);
    std::unique_ptr<output_port> port3 = std::make_unique<output_port>(
            port3_pin, ad57x4::CHANNEL_D, dac1);
    output_demux demux0(output_demux::OVERWRITE_OLDEST);

    TwoWire disp_i2c(PB11, PB10);
    Adafruit_SSD1306 display(128, 64, &disp_i2c, -1);
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

void rot_clk_isr() {
    using namespace midimagic;
    volatile int i = digitalRead(rot_dt);
    volatile int j;
    if (i == HIGH)
        j = 0xdead;
    if (i == LOW)
        j = 0xbeef;
}

void rot_sw_isr() {
    using namespace midimagic;
    int i = 0xbeef;
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
    demux0.add_output(std::move(port2));
    demux0.add_output(std::move(port3));
    pinMode(rot_dt, INPUT_PULLUP);
    pinMode(rot_clk, INPUT_PULLUP);
    pinMode(rot_sw, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(rot_clk), rot_clk_isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(rot_sw), rot_sw_isr, FALLING);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println(F("midimagic"));
    display.setTextSize(1);
    display.println(F("by"));
    display.println(F("raumschiffgeraeusche"));
    display.display();
}

void loop() {
    using namespace midimagic;
    MIDI.read();
}