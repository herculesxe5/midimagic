#include "rotary.h"

namespace midimagic {

    void rotary::signal_clk() {
        m_rot_dtstate = digitalRead(rot_dt);
        m_current_millis = millis();
        if (m_current_millis - m_rot_clk_ts > k_rot_int_th) {
            m_rot_clk_ts = m_current_millis;

            if (m_rot_dtstate == HIGH) {
                menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_LEFT);
                menu->notify(a);
            }
            else if (m_rot_dtstate == LOW) {
                menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_RIGHT);
                menu->notify(a);
            }
        }
    }

    void rotary::signal_sw() {
        m_rot_swstate = digitalRead(rot_sw);
        m_current_millis = millis();
        if (m_rot_swstate == LOW) {
            m_rot_sw_ts = m_current_millis;
        } else {
            if (m_current_millis - m_rot_sw_ts > k_rot_lp) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_BUTTON_LONGPRESS);
                menu->notify(a);
            }
            else if (m_current_millis - m_rot_sw_ts > k_rot_int_th) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_BUTTON);
                menu->notify(a);
            }
        }
    }
}
