/******************************************************************************
 *                                                                            *
 * Copyright 2020 Lukas Jünger                                                *
 *                                                                            *
 * This file is part of Midimagic.                                            *
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

#ifndef AD57X4_H
#define AD57X4_H
#include "common.h"
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

        explicit ad57x4(SPIClass &spi, u8 sync);
        ad57x4()= delete;
        ad57x4(const ad57x4&) = delete;
        ~ad57x4();

        void set_level(u16 level, u8 channel);

    private:
        SPIClass &m_spi;
        u8 m_sync;

        void send(u8 (&data)[3]);
    };
}
#endif  //AD57X4_H
