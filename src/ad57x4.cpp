#include "ad57x4.h"
#define REG_OUTPUT_RANGE_MASK 0x08
#define REG_POWER_CTRL_MASK   0x10

namespace midimagic {
ad57x4::ad57x4(SPIClass &spi, u8 sync, u8 ldac) :
    m_spi(spi),
    m_sync(sync),
    m_ldac(ldac) {
  m_spi.begin(m_sync);
  digitalWrite(m_ldac, HIGH);

  // set output range to 5V unipolar
  m_spi.transfer(m_sync, REG_OUTPUT_RANGE_MASK | ALL_CHANNELS, SPI_CONTINUE);
  m_spi.transfer16(m_sync, 0x0, SPI_LAST);

  // power all channels
  m_spi.transfer(m_sync, REG_POWER_CTRL_MASK, SPI_CONTINUE);
  m_spi.transfer(m_sync, 0x0, SPI_CONTINUE);
  m_spi.transfer(m_sync, 0x0F, SPI_LAST);

  delay(1);
  digitalWrite(m_ldac, LOW);
}

ad57x4::~ad57x4() {
    //nothing to do
}

void ad57x4::set_level(u16 level, u8 channel) {
  m_spi.transfer(m_sync, channel, SPI_CONTINUE);
  m_spi.transfer16(m_sync, level, SPI_LAST);
}
}