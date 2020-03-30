#ifndef AD57X4_H
#define AD57X4_H
#include <common.h>
#include <SPI.h>

namespace midimagic {
    class ad57x4 {
    public:
        enum dac_channel {
            CHANNEL_A,
            CHANNEL_B,
            CHANNEL_C,
            CHANNEL_D,
            ALL_CHANNELS
        };

        ad57x4(SPIClass &spi, u8 sync, u8 ldac);
        ad57x4()= delete;
        ad57x4(const ad57x4&) = delete;
        ~ad57x4();

        void set_level(u16 level, u8 out);

    private:
        SPIClass &m_spi;
        u8 m_sync;
        u8 m_ldac;

        void send(u8 (&data)[3]);
    };
}
#endif  //AD57X4_H