/******************************************************************************
 *                                                                            *
 * Copyright 2022 Lukas Jünger and Adrian Krause                              *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify it    *
 * under the terms of the GNU Lesser General Public License as published by   *
 * the Free Software Foundation, either version 3 of the License,             *
 * or (at your option) any later version.                                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.      *
 *                                                                            *
 ******************************************************************************/

#include <SPI.h>
#include <MIDI.h>
#include <Wire.h>

#include "common.h"
#include "hardware_config.h"
#include "ad57x4.h"
#include "midi_types.h"
#include "output.h"
#include "menu.h"
#include "rotary.h"
#include <lcdgfx.h>
#include "bitmaps.h"
#include "port_group.h"
#include "inventory.h"

namespace midimagic {

    SPIClass spi1(hw_setup.dac.mosi, hw_setup.dac.miso, hw_setup.dac.clk);

    ad57x4 dac0(spi1, hw_setup.dac.cs0);
    ad57x4 dac1(spi1, hw_setup.dac.cs1);

    midi::MidiInterface<HardwareSerial> MIDI((HardwareSerial&)Serial1);

    std::shared_ptr<group_dispatcher> port_master(new group_dispatcher);
    std::shared_ptr<menu_state> menu(new menu_state);
    std::shared_ptr<menu_action_queue> action_queue(new menu_action_queue(menu));

    std::shared_ptr<inventory> invent(new inventory(port_master, action_queue, dac0, dac1));

    rotary rot(hw_setup.rotary.dat, hw_setup.rotary.swi, action_queue);

    const SPlatformI2cConfig display_config = (SPlatformI2cConfig)
                                            { .busId = 2,
                                              .addr = hw_setup.display.addr,
                                              .scl = hw_setup.display.sclk,
                                              .sda = hw_setup.display.sdat,
                                              .frequency = 0 };

    DisplaySSD1306_128x64_I2C display(-1, display_config);
};

void handleNoteOn(byte midi_channel, byte midi_note, byte midi_velo) {
    using namespace midimagic;
    midi_message msg(midi_message::NOTE_ON, midi_channel, midi_note, midi_velo);
    port_master->add_message(msg);
}

void handleNoteOff(byte midi_channel, byte midi_note, byte midi_velo) {
    using namespace midimagic;
    midi_message msg(midi_message::NOTE_OFF, midi_channel, midi_note, midi_velo);
    port_master->add_message(msg);
}

void handleAfterTouchPoly(byte midi_channel, byte midi_note, byte pressure) {
    using namespace midimagic;
    midi_message msg(midi_message::POLY_KEY_PRESSURE, midi_channel, midi_note, pressure);
    port_master->add_message(msg);
}

void handleControlChange(byte midi_channel, byte midi_control_number, byte value) {
    using namespace midimagic;
    midi_message msg(midi_message::CONTROL_CHANGE, midi_channel, midi_control_number, value);
    port_master->add_message(msg);
}

void handleProgramChange(byte midi_channel, byte midi_program_number) {
    using namespace midimagic;
    midi_message msg(midi_message::PROGRAM_CHANGE, midi_channel, midi_program_number, 0);
    port_master->add_message(msg);
}

void handleAfterTouchChannel(byte midi_channel, byte pressure) {
    using namespace midimagic;
    midi_message msg(midi_message::CHANNEL_PRESSURE, midi_channel, pressure, 0);
    port_master->add_message(msg);
}

void handlePitchBend(byte midi_channel, int pitch_offset) {
    using namespace midimagic;
    // Handle possible negative pitch offset
    // Library expects 16-bit ints, but stm32 uses 32-bit width,
    // so ignoring half of the bits should be ok
    u16 mangled_offset = (u16) (pitch_offset & 0xffff);
    if (pitch_offset < 0) {
        // add leading 1 if negative
        mangled_offset = mangled_offset | 0x8000;
    }
    // need to transport 16 bit integer
    // midi_message.data0 holds MSB, data1 holds LSB
    midi_message msg(midi_message::PITCH_BEND, midi_channel, (u8) (mangled_offset >> 8), (u8) (mangled_offset & 0xff));
    port_master->add_message(msg);
}

void handleClock() {
    using namespace midimagic;
    midi_message msg(midi_message::message_type::CLOCK, 0, 0, 0);
    port_master->add_message(msg);
}

void handleStart() {
    using namespace midimagic;
    midi_message msg(midi_message::message_type::START, 0, 0, 0);
    port_master->add_message(msg);
}

void handleContinue() {
    using namespace midimagic;
    midi_message msg(midi_message::message_type::CONTINUE, 0, 0, 0);
    port_master->add_message(msg);
}

void handleStop() {
    using namespace midimagic;
    midi_message msg(midi_message::message_type::STOP, 0, 0, 0);
    port_master->add_message(msg);
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
    // Add delay to leave setup time for the display and DACs
    delay(500);
    // Power up dacs
    pinMode(hw_setup.dac.power, OUTPUT);
    digitalWrite(hw_setup.dac.power, HIGH);
    dac0.set_level(0, ad57x4::ALL_CHANNELS);
    dac1.set_level(0, ad57x4::ALL_CHANNELS);
    // Set up MIDI
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleAfterTouchPoly(handleAfterTouchPoly);
    MIDI.setHandleControlChange(handleControlChange);
    MIDI.setHandleProgramChange(handleProgramChange);
    MIDI.setHandleAfterTouchChannel(handleAfterTouchChannel);
    MIDI.setHandlePitchBend(handlePitchBend);
    MIDI.setHandleClock(handleClock);
    MIDI.setHandleStart(handleStart);
    MIDI.setHandleContinue(handleContinue);
    MIDI.setHandleStop(handleStop);
    MIDI.begin(MIDI_CHANNEL_OMNI);

    // Set up interrupts
    pinMode(hw_setup.rotary.dat, INPUT_PULLUP);
    pinMode(hw_setup.rotary.clk, INPUT_PULLUP);
    pinMode(hw_setup.rotary.swi, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(hw_setup.rotary.clk), rot_clk_isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(hw_setup.rotary.swi), rot_sw_isr, CHANGE);

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

    // Try to load config from eeprom
    auto return_code = invent->load_config_from_eeprom();
    if (return_code != config_archive::operation_result::SUCCESS) {
        display.clear();
        display.printFixed(0, 8, "Config load error:");
        display.setTextCursor(114, 8);
        display.print(return_code);
        delay(3000);
    }

    // Show menu
    auto v = std::make_shared<over_view>(display, menu, invent);
    menu->register_view(v);
}

void loop() {
    using namespace midimagic;
    MIDI.read();
    action_queue->exec_next_action();
}
