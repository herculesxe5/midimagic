#include "menu.h"

namespace midimagic {

    menu_action::menu_action(kind k, subkind sk, int d0, int d1)
        : m_kind(k)
        , m_subkind(sk)
        , m_data0(d0)
        , m_data1(d1) {
        // nothing to do
    }

    menu_action::~menu_action() {
        // nothing to do
    }

    menu_view::menu_view(Adafruit_SSD1306 &d)
        : m_display(d) {
        // nothing to do
    }

    menu_view::~menu_view() {
        // nothing to do
    }

    pin_view::pin_view(u8 pin, Adafruit_SSD1306 &d)
        : menu_view(d)
        , m_pin(pin) {
        // nothing to do
    }

    pin_view::~pin_view() {
        // nothing to do
    }

    void pin_view::notify(const menu_action &a) const {

    }

    over_view::over_view(Adafruit_SSD1306 &d)
        : menu_view(d)
        , m_rowoffset(32)
        , m_activitydot_xoffset(4)
        , m_port_value_xoffset(12)
        , m_activity_yoffset(20) {
        // nothing to do
    }

    over_view::~over_view() {
        // nothing to do
    }

    void over_view::notify(const menu_action &a) const {

        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clearDisplay();
                draw_statics();
                m_display.display();
                break;
            case menu_action::kind::PORT_ACTIVITY :
                if (a.m_subkind == menu_action::subkind::PORT_ACTIVE) {
                    draw_activity(a.m_data0, a.m_data1);
                }
                if (a.m_subkind == menu_action::subkind::PORT_NACTIVE) {
                    draw_inactivity(a.m_data0);
                }
                m_display.display();
                break;
            default :
                // nothing to do
                break;
        }
    }

    void over_view::draw_statics() const {
        // Draw frame
        m_display.drawFastHLine(  0,  0, 128, SSD1306_WHITE);
        m_display.drawFastHLine(  0, 63, 128, SSD1306_WHITE);
        m_display.drawFastVLine(  0,  0,  64, SSD1306_WHITE);
        m_display.drawFastVLine(127,  0,  64, SSD1306_WHITE);
        // Draw crosses
        m_display.drawFastHLine( 0, 31, 128, SSD1306_WHITE);
        m_display.drawFastVLine(31,  0,  64, SSD1306_WHITE);
        m_display.drawFastVLine(64,  0,  64, SSD1306_WHITE);
        m_display.drawFastVLine(97,  0,  64, SSD1306_WHITE);

        // Fill cells with port numbers
        m_display.setTextSize(1);
        const uint8_t N_xoffset = 4;
        const uint8_t cellborder_yoffset = 4;

        for (uint8_t i=0; i<8; i++) {
            if (i < 4) {
                m_display.setCursor(i*32+N_xoffset, cellborder_yoffset);
            } else {
                m_display.setCursor((i-4)*32+N_xoffset, cellborder_yoffset+m_rowoffset);
            }
            m_display.print(i+1);
        }
    }

    void over_view::draw_activity(const int port_number, const int port_value) const {
        m_display.setTextSize(1);
        int xoffset;
        int yoffset;

        // calculate offsets
        if (port_number < 4) {
            xoffset = port_number*32;
            yoffset = m_activity_yoffset;
        } else {
            xoffset = (port_number-4)*32;
            yoffset = m_activity_yoffset + m_rowoffset;
        }

        // draw activity dot
        m_display.setCursor(xoffset+m_activitydot_xoffset, yoffset);
        m_display.print("*");

        // draw port value
        m_display.fillRect(xoffset+m_port_value_xoffset, yoffset, 12, 10, SSD1306_BLACK);
        m_display.setCursor(xoffset+m_port_value_xoffset, yoffset);
        m_display.print(port_value);

    }

    void over_view::draw_inactivity(const int port_number) const {

        // draw black rectangle over activity dot
        if (port_number < 4) {
            m_display.fillRect(port_number*32+m_activitydot_xoffset, m_activity_yoffset, 5, 10, SSD1306_BLACK);
        } else {
            m_display.fillRect((port_number-4)*32+m_activitydot_xoffset, m_activity_yoffset+m_rowoffset, 5, 10, SSD1306_BLACK);
        }
    }

    menu_state::menu_state()
        : m_rot_int_ts(0)
        , m_view() {
            // nothing to do
    }

    void menu_state::register_view(std::shared_ptr<menu_view> m) {
        m_view = m;
        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
        m_view->notify(a);
    }

    void menu_state::notify(const menu_action &a) {
        m_view->notify(a);
    }
}
