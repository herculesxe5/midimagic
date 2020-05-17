#include "ad57x4.h"
#define REG_OUTPUT_RANGE_MASK 0x08
#define REG_POWER_CTRL_MASK   0x10

namespace midimagic {
ad57x4::ad57x4(SPIClass &spi, u8 sync) :
    m_spi(spi),
    m_sync(sync) {
  m_spi.begin(m_sync);
  // set output range +-5V
  u8 data[3];
  data[0] = REG_OUTPUT_RANGE_MASK | ALL_CHANNELS;
  data[1] = 0x0;
  data[2] = 0b11;
  send(data);
  // power all channels
  data[0] = REG_POWER_CTRL_MASK;
  data[1] = 0x0;
  data[2] = 0x0F;
  send(data);
}

ad57x4::~ad57x4() {
    //nothing to do
}

void ad57x4::set_level(u16 level, u8 channel) {
   u8 data[3];
   data[0] = channel;
   data[1] = (u8) (level >> 8) & 0xFF;
   data[2] = (u8) level & 0xFF;
   send(data);
}

void ad57x4::send(u8 (&data)[3]) {
    m_spi.transfer(m_sync, data[0], SPI_CONTINUE);
    m_spi.transfer16(m_sync, data[1] << 8 | data[2], SPI_LAST);
}
} // namespace midimagic