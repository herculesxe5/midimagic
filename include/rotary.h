#ifndef MIDIMAGIC_ROTARY_H
#define MIDIMAGIC_ROTARY_H

#include "common.h"

#include "menu.h"

namespace midimagic {

    extern const u8 rot_dt;
    extern const u8 rot_sw;
    extern std::shared_ptr<menu_state> menu;


    class rotary {
    public:
        void signal_clk();
        void signal_sw();

    private:
        volatile int m_rot_dtstate;
        volatile int m_rot_swstate;
        volatile unsigned long m_current_millis;

        //Rotary encoder ISR timestamps
        volatile unsigned long m_rot_clk_ts;
        volatile unsigned long m_rot_sw_ts;
        //Rotary encoder threshold, ms
        const unsigned int k_rot_int_th = 50;
        //Rotary switch long press duration, ms
        const unsigned int k_rot_lp = 1000;
    };
}



#endif //MIDIMAGIC_ROTARY_H
