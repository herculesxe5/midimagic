#include <common.h>
#include <SPI.h>

#include "ad57x4.h"

namespace midimagic {

const u8 cs_dac = PA0;
const u8 latch_dac = PB3;
const u8 mosi_dac = PA7;
const u8 miso_dac = PA6;
const u8 clk_dac = PA5;

SPIClass spi1(mosi_dac, miso_dac, clk_dac);
ad57x4 dac(spi1, cs_dac, latch_dac);

};

using namespace midimagic;

void setup() {
}

void loop() {
  dac.set_level(0x0000, ad57x4::ALL_CHANNELS);
  delay(2000);
  dac.set_level(0xFFFF, ad57x4::ALL_CHANNELS);
  delay(2000);
}