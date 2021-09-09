#ifndef MIDIMAGIC_ROTARY_H
#define MIDIMAGIC_ROTARY_H

#include "common.h"
#include "menu_action_queue.h"
#include <memory>

namespace midimagic {

    class rotary {
    public:
        explicit rotary(const u8 data_pin, const u8 switch_pin,
                        std::shared_ptr<menu_action_queue> aq);
        rotary() = delete;
        rotary(const rotary&) = delete;
        ~rotary();

        void signal_clk();
        void signal_sw();

    private:
        const u8 m_data_pin;
        const u8 m_switch_pin;
        std::shared_ptr<menu_action_queue> m_action_queue;

        volatile int m_rot_dtstate;
        volatile int m_rot_swstate;
        volatile unsigned long m_current_millis;

        //Rotary encoder ISR timestamps
        volatile unsigned long m_rot_clk_ts;
        volatile unsigned long m_rot_sw_ts;
        //Rotary encoder threshold, ms
        const unsigned int k_rot_int_th = 50;
        //Rotary switch long press duration, ms
        const unsigned int k_rot_lp = 500;
    };
}



#endif //MIDIMAGIC_ROTARY_H
