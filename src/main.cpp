#include <SPI.h>
#include <MIDI.h>
#include <Wire.h>

#include "common.h"
#include "ad57x4.h"
#include "midi_types.h"
#include "output.h"
#include "menu.h"
#include "rotary.h"
#include <lcdgfx.h>
#include "bitmaps.h"

namespace midimagic {

    const u8 power_dacs = PB12;
    const u8 cs_dac0    = PA0;
    const u8 cs_dac1    = PA1;
    const u8 mosi_dac   = PA7;
    const u8 miso_dac   = PA6;
    const u8 clk_dac    = PA5;
    const u8 rot_sw     = PB15;
    const u8 rot_clk    = PB14;
    const u8 rot_dt     = PB13;
    const u8 disp_scl   = PB10;
    const u8 disp_sca   = PB11;
    const u8 port0_pin  = PB9;
    const u8 port1_pin  = PB8;
    const u8 port2_pin  = PB7;
    const u8 port3_pin  = PB6;
    const u8 port4_pin  = PB5;
    const u8 port5_pin  = PB4;
    const u8 port6_pin  = PB3;
    const u8 port7_pin  = PA15;

    SPIClass spi1(mosi_dac, miso_dac, clk_dac);

    ad57x4 dac0(spi1, cs_dac0);
    ad57x4 dac1(spi1, cs_dac1);

    midi::MidiInterface<HardwareSerial> MIDI((HardwareSerial&)Serial1);

    std::shared_ptr<menu_state> menu(new menu_state);
    /*
    output_port port0(port0_pin, ad57x4::CHANNEL_A, dac1, menu, 0);
    output_port port1(port1_pin, ad57x4::CHANNEL_B, dac1, menu, 1);
    output_port port2(port2_pin, ad57x4::CHANNEL_C, dac1, menu, 2);
    output_port port3(port3_pin, ad57x4::CHANNEL_D, dac1, menu, 3);
    output_port port4(port4_pin, ad57x4::CHANNEL_A, dac0, menu, 4);
    output_port port5(port5_pin, ad57x4::CHANNEL_B, dac0, menu, 5);
    output_port port6(port6_pin, ad57x4::CHANNEL_C, dac0, menu, 6);
    output_port port7(port7_pin, ad57x4::CHANNEL_D, dac0, menu, 7);

    identic_output_demux demux0;
    */
    rotary rot(rot_dt, rot_sw, menu);

    const SPlatformI2cConfig display_config = (SPlatformI2cConfig)
                                          { .busId = 2,
                                            .addr = 0x3C,
                                            .scl = disp_scl,
                                            .sda = disp_sca,
                                            .frequency = 0 };

    DisplaySSD1306_128x64_I2C display(-1, display_config);

};

void handleNoteOn(byte midi_channel, byte midi_note, byte midi_velo) {
    using namespace midimagic;
    midi_message msg(midi_message::NOTE_ON, midi_channel, midi_note, midi_velo);
    //FIXME call port_group dispatcher
    //demux0.add_note(msg);
}

void handleNoteOff(byte midi_channel, byte midi_note, byte midi_velo) {
    using namespace midimagic;
    midi_message msg(midi_message::NOTE_OFF, midi_channel, midi_note, midi_velo);
    //FIXME call port_group dispatcher
    //demux0.remove_note(msg);
}

void rot_clk_isr() {
    using namespace midimagic;
    rot.signal_clk();
}

void rot_sw_isr() {
    using namespace midimagic;
    rot.signal_sw();
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
    /*demux0.add_output(port0);
    demux0.add_output(port1);
    demux0.add_output(port2);
    demux0.add_output(port3);
    demux0.add_output(port4);
    demux0.add_output(port5);
    demux0.add_output(port6);
    demux0.add_output(port7);*/
    // Set up interrupts
    pinMode(rot_dt, INPUT_PULLUP);
    pinMode(rot_clk, INPUT_PULLUP);
    pinMode(rot_sw, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(rot_clk), rot_clk_isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(rot_sw), rot_sw_isr, CHANGE);
    // Display boot screen
    display.begin();
    display.clear();
    display.setFixedFont(ssd1306xled_font8x16);
    display.printFixed(0, 0, "midimagic", STYLE_BOLD);
    display.setFixedFont(ssd1306xled_font6x8);
    display.printFixed(0, 34, "by", STYLE_ITALIC);
    display.printFixed(0, 42, "raumschiffgeraeusche", STYLE_ITALIC);
    // Wait 3 sec before display refresh
    delay(3000);
    auto v = std::make_shared<over_view>(display, menu);
    menu->register_view(v);
}

void loop() {
    using namespace midimagic;
    MIDI.read();
}
