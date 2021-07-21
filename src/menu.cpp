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

    menu_view::menu_view(DisplaySSD1306_128x64_I2C &d, std::shared_ptr<menu_state> menu_state)
        : m_display(d)
        , m_menu_state(menu_state) {
        // nothing to do
    }

    menu_view::~menu_view() {
        // nothing to do
    }

    pin_view::pin_view(u8 pin, DisplaySSD1306_128x64_I2C &d, std::shared_ptr<menu_state> menu_state)
        : menu_view(d, menu_state)
        , m_pin(pin)
        , m_menu_items({"menu item 1",
                        "menu item 2",
                        "menu item 3",
                        "menu item 4",
                        "menu item 5",
                        "menu item 6"})
        , m_pin_menu_dimensions({NanoPoint{0, 8}, NanoPoint{127, 63}})
        {
        pin_menu = std::make_shared<LcdGfxMenu>(m_menu_items, 6, m_pin_menu_dimensions);
    }

    pin_view::~pin_view() {
        // nothing to do
    }

    void pin_view::notify(const menu_action &a) {

        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                // show pin submenu
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Pin:", STYLE_NORMAL);
                m_display.setTextCursor(30, 0);
                m_display.print(m_pin);
                pin_menu->show(m_display);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    //FIXME do pin_view stuff
                    //call pin_menu->selection(); to get the selected item
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // switch to over_view
                    auto v = std::make_shared<over_view>(m_display, m_menu_state);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    pin_menu->down();
                    pin_menu->show(m_display);
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    pin_menu->up();
                    pin_menu->show(m_display);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    over_view::over_view(DisplaySSD1306_128x64_I2C &d, std::shared_ptr<menu_state> menu_state)
        : menu_view(d, menu_state)
        , m_pin_select(1)
        , m_rowoffset(32)
        , m_activitydot_xoffset(4)
        , m_port_value_xoffset(12)
        , m_activity_yoffset(17)
        , m_selection_xcelloffset(25)
        , m_selection_yoffset(8) {
        // nothing to do
    }

    over_view::~over_view() {
        // nothing to do
    }

    void over_view::notify(const menu_action &a) {

        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font5x7);
                draw_statics();
                draw_pin_select();
                break;
            case menu_action::kind::PORT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::PORT_ACTIVE) {
                    draw_activity(a.m_data0, a.m_data1);
                } else if (a.m_subkind == menu_action::subkind::PORT_NACTIVE) {
                    draw_inactivity(a.m_data0);
                }
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    // switch to pin_view
                    auto v = std::make_shared<pin_view>(m_pin_select, m_display, m_menu_state);
                    m_menu_state->register_view(v);

                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    //FIXME switch to main menu

                } else if (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    draw_pin_deselect();
                    // select next pin
                    m_pin_select++;
                    if (m_pin_select > 8) {
                        m_pin_select = 1;
                    }
                    draw_pin_select();

                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    draw_pin_deselect();
                    // select previous pin
                    m_pin_select--;
                    if (m_pin_select < 1) {
                        m_pin_select = 8;
                    }
                    draw_pin_select();
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    void over_view::draw_statics() const {
        m_display.drawBitmap1(0, 0, 128, 64, over_view_bg);
    }

    void over_view::draw_pin_select() const {
        // draw selection marker bitmap
        if (m_pin_select < 5) {
            m_display.drawBitmap1((m_pin_select-1)*32+m_selection_xcelloffset,
                                m_selection_yoffset, sizeof(arrow_down), 8, arrow_down);
        } else {
            m_display.drawBitmap1((m_pin_select-1-4)*32+m_selection_xcelloffset,
                                m_selection_yoffset+m_rowoffset, sizeof(arrow_down), 8, arrow_down);
        }
    }

    void over_view::draw_pin_deselect() const {
        // overwrite selection marker with black bitmap
        if (m_pin_select < 5) {
            m_display.drawBitmap1((m_pin_select-1)*32+m_selection_xcelloffset,
                                m_selection_yoffset, sizeof(black_rect_5x8), 8, black_rect_5x8);
        } else {
            m_display.drawBitmap1((m_pin_select-1-4)*32+m_selection_xcelloffset,
                                m_selection_yoffset+m_rowoffset, sizeof(black_rect_5x8), 8, black_rect_5x8);
        }
    }

    void over_view::draw_activity(const int port_number, const int port_value) const {
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

        m_display.setFixedFont(ssd1306xled_font5x7);
        // draw activity dot
        m_display.printFixed(xoffset+m_activitydot_xoffset, yoffset, "*", STYLE_NORMAL);

        // draw port value
        m_display.printFixed(xoffset+m_port_value_xoffset, yoffset, "   ", STYLE_NORMAL);
        m_display.setTextCursor(xoffset+m_port_value_xoffset, yoffset);
        m_display.print(port_value);
    }

    void over_view::draw_inactivity(const int port_number) const {
        m_display.setFixedFont(ssd1306xled_font5x7);
        // draw black rectangle over activity dot
        if (port_number < 4) {
            m_display.printFixed(port_number*32+m_activitydot_xoffset, m_activity_yoffset, " ", STYLE_NORMAL);
        } else {
            m_display.printFixed((port_number-4)*32+m_activitydot_xoffset, m_activity_yoffset+m_rowoffset, " ", STYLE_NORMAL);
        }
    }

    menu_state::menu_state()
        : m_view() {
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
