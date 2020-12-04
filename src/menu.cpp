#include "menu.h"

namespace midimagic {
    menu_view::menu_view(menu_state &s , Adafruit_SSD1306 &d)
        : m_state(s)
        , m_display(d) {
        // nothing to do
    }

    menu_view::~menu_view() {
        // nothing to do
    }

    void menu_view::notify() const {

    };

    pin_view::pin_view(u8 pin, menu_state& s , Adafruit_SSD1306 &d)
        : m_pin(pin)
        , menu_view(s, d) {
        // nothing to do
    }

    pin_view::~pin_view() {
        // nothing to do
    }

    void pin_view::notify() const {

    }

    over_view::over_view(menu_state& s, Adafruit_SSD1306 &d)
        : menu_view(s, d) {
        // nothing to do
    }

    over_view::~over_view() {
        // nothing to do
    }

    void over_view::notify() const {
        m_display.clearDisplay();
        m_display.setTextSize(2);
        m_display.setCursor(0, 0);
        m_display.println(m_state.m_count);
        m_display.display();
    }

    menu_state::menu_state()
        : m_count(0)
        , m_view() {
            // nothing to do
    }

    void menu_state::register_view(std::shared_ptr<menu_view> m) {
        m_view = m;
        m_view->notify();
    }

    void menu_state::notify(const rot_action a) {
        switch(a) {
            case ROT_LEFT:
                m_count++;
                break;
            case ROT_RIGHT:
                m_count--;
                break;
            case ROT_BUTTON:
                m_count = 0;
                break;
        }
        m_view->notify();
    }
}