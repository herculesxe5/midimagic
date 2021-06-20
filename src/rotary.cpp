#include "rotary.h"

namespace midimagic {

    rotary::rotary(const u8 data_pin, const u8 switch_pin, std::shared_ptr<menu_state> ms)
        : m_data_pin(data_pin)
        , m_switch_pin(switch_pin)
        , m_menu_state(ms) {
        // nothing to do
    }

    rotary::~rotary() {
        // nothing to do
    }

    void rotary::signal_clk() {
        m_rot_dtstate = digitalRead(m_data_pin);
        m_current_millis = millis();
        if (m_current_millis - m_rot_clk_ts > k_rot_int_th) {
            m_rot_clk_ts = m_current_millis;

            if (m_rot_dtstate == HIGH) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_LEFT);
                m_menu_state->notify(a);
            }
            else if (m_rot_dtstate == LOW) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_RIGHT);
                m_menu_state->notify(a);
            }
        }
    }

    void rotary::signal_sw() {
        m_rot_swstate = digitalRead(m_switch_pin);
        m_current_millis = millis();
        if (m_rot_swstate == LOW) {
            m_rot_sw_ts = m_current_millis;
        } else {
            if (m_current_millis - m_rot_sw_ts > k_rot_lp) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_BUTTON_LONGPRESS);
                m_menu_state->notify(a);
            }
            else if (m_current_millis - m_rot_sw_ts > k_rot_int_th) {
                const menu_action a(menu_action::kind::ROT_ACTIVITY, menu_action::subkind::ROT_BUTTON);
                m_menu_state->notify(a);
            }
        }
    }
}
