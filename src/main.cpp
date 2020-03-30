#include <common.h>
#include <SPI.h>

#include "ad57x4.h"

namespace midimagic {
  const u8 cs_dac0 = PA0;
  const u8 cs_dac1 = PA1;
  const u8 latch_dac = PB3;
  const u8 mosi_dac = PA7;
  const u8 miso_dac = PA6;
  const u8 clk_dac = PA5;
  SPIClass spi1(mosi_dac, miso_dac, clk_dac);

  ad57x4 *dac0 = new ad57x4(spi1, cs_dac0, latch_dac);
  ad57x4 *dac1 = new ad57x4(spi1, cs_dac1, latch_dac);
  ad57x4 *dacs[2] = {dac0, dac1};
};

void setup() {
  using namespace midimagic;
}

void loop() {
  using namespace midimagic;
  int8_t channel = ad57x4::CHANNEL_D;
  while (channel >= 0) {
      dacs[0]->set_level(0x0000, channel);
      dacs[1]->set_level(0x0000, channel);
      delay(1000);
      dacs[0]->set_level(0xFFFF, channel);
      dacs[1]->set_level(0xFFFF, channel);
      delay(1000);
      channel--;
  }
}