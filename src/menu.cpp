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

    menu_view::menu_view(DisplaySSD1306_128x64_I2C &d,
                         std::shared_ptr<menu_state> menu_state,
                         std::shared_ptr<inventory> invent)
        : m_display(d)
        , m_menu_state(menu_state)
        , m_inventory(invent) {
        // nothing to do
    }

    menu_view::~menu_view() {
        // nothing to do
    }

    port_view::port_view(u8 port_number,
                       DisplaySSD1306_128x64_I2C &d,
                       std::shared_ptr<menu_state> menu_state,
                       std::shared_ptr<inventory> invent)
        : menu_view(d, menu_state, invent)
        , m_port_number(port_number)
        , m_port(m_inventory->get_output_port(m_port_number))
        , m_menu_items{"Switch Note/Velocit",
                       "Change Clock Rate",
                       "Resync Clock"}
        , m_port_menu_dimensions{NanoPoint{0, 24}, NanoPoint{127, 63}}
        {
        m_port_menu = std::make_unique<LcdGfxMenu>(m_menu_items,
            sizeof(m_menu_items) / sizeof(char *), m_port_menu_dimensions);
    }

    port_view::~port_view() {
        // nothing to do
    }

    void port_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                // show pin submenu
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(0, 0, "Port:", STYLE_NORMAL);
                m_display.setTextCursor(36, 0);
                m_display.print(m_port_number + 1);
                if (m_port->get_velocity_switch()) {
                    m_display.printFixed(0, 8, "Output: Velocity", STYLE_NORMAL);
                } else {
                    m_display.printFixed(0, 8, "Output: Note", STYLE_NORMAL);
                }
                switch (m_port->get_clock_rate()) {
                    case 12 :
                        m_display.printFixed(0, 16, "Clock Rate: 1/8");
                        break;
                    case 24 :
                        m_display.printFixed(0, 16, "Clock Rate: 1/4");
                        break;
                    case 48 :
                        m_display.printFixed(0, 16, "Clock Rate: 1/2");
                        break;
                    case 96 :
                        m_display.printFixed(0, 16, "Clock Rate: 1");
                        break;
                    default :
                        m_display.printFixed(0, 16, "Clock Rate:");
                        m_display.setTextCursor(72, 16);
                        m_display.print(m_port->get_clock_rate());
                        break;
                }
                m_port_menu->show(m_display);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    if (m_port_menu->selection() == 0) {
                            m_port->set_velocity_switch();
                            // inform inventory about change
                            m_inventory->submit_output_port_change(m_port_number);
                            // trigger display update
                            menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                            m_menu_state->notify(a);
                    } else if (m_port_menu->selection() == 1) {
                            // switch to config_port_clock_view
                            auto v = std::make_shared<config_port_clock_view>(m_port_number, m_display, m_menu_state, m_inventory);
                            m_menu_state->register_view(v);
                    } else if (m_port_menu->selection() == 2) {
                            m_port->reset_clock();
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // switch to over_view
                    auto v = std::make_shared<over_view>(m_display, m_menu_state, m_inventory);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    m_port_menu->down();
                    m_port_menu->show(m_display);
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    m_port_menu->up();
                    m_port_menu->show(m_display);
                }
                break;
            case menu_action::kind::PORT_ACTIVITY :
                if (a.m_data0 == m_port_number) {
                    if        (a.m_subkind == menu_action::subkind::PORT_ACTIVE) {
                        m_display.printFixed(100, 0, "*", STYLE_NORMAL);
                        m_display.setTextCursor(108, 0);
                        m_display.print(a.m_data1);
                    } else if (a.m_subkind == menu_action::subkind::PORT_NACTIVE) {
                        m_display.printFixed(100, 0, " ", STYLE_NORMAL);
                    }
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_port_clock_view::config_port_clock_view(u8 port_number,
                                                   DisplaySSD1306_128x64_I2C &d,
                                                   std::shared_ptr<menu_state> menu_state,
                                                   std::shared_ptr<inventory> invent)
        : port_view(port_number, d, menu_state, invent)
        , m_clock_rate(m_port->get_clock_rate()) {
        // nothing to do
    }

    config_port_clock_view::~config_port_clock_view() {
        // nothing to do
    }

    void config_port_clock_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Port:", STYLE_NORMAL);
                m_display.setTextCursor(36, 0);
                m_display.print(m_port_number + 1);
                m_display.printFixed(0, 8, "Set Port Clock Rate:", STYLE_NORMAL);
                switch (m_clock_rate) {
                    case 12 :
                        m_display.printFixed(0, 16, "1/8 Note");
                        break;
                    case 24 :
                        m_display.printFixed(0, 16, "1/4 Note");
                        break;
                    case 48 :
                        m_display.printFixed(0, 16, "1/2 Note");
                        break;
                    case 96 :
                        m_display.printFixed(0, 16, "1 Note");
                        break;
                    default :
                        m_display.setTextCursor(0, 16);
                        m_display.print(m_clock_rate);
                        m_display.printFixed(32, 16, "timing Clocks");
                        break;
                }
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    if (m_clock_rate % 12) {
                        m_clock_rate = 12;
                    } else if (m_clock_rate < 96) {
                        m_clock_rate = m_clock_rate * 2;
                    }
                    // trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);

                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    if (m_clock_rate % 12) {
                        m_clock_rate = 12;
                    } else if (m_clock_rate > 12) {
                        m_clock_rate = m_clock_rate / 2;
                    }
                    // trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);

                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    m_port->set_clock_rate(m_clock_rate);
                    // inform inventory about change
                    m_inventory->submit_output_port_change(m_port_number);
                    // switch back to port_view
                    auto v = std::make_shared<port_view>(m_port_number, m_display, m_menu_state, m_inventory);
                    m_menu_state->register_view(v);

                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // switch back to port_view without setting clock rate
                    auto v = std::make_shared<port_view>(m_port_number, m_display, m_menu_state, m_inventory);
                    m_menu_state->register_view(v);
                }
            default:
                // nothing to do
                break;
        }
    }

    over_view::over_view(DisplaySSD1306_128x64_I2C &d,
                         std::shared_ptr<menu_state> menu_state,
                         std::shared_ptr<inventory> invent)
        : menu_view(d, menu_state, invent)
        , m_pin_select(0)
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
                    // Switch to port_view
                    auto v = std::make_shared<port_view>(m_pin_select, m_display, m_menu_state, m_inventory);
                    m_menu_state->register_view(v);

                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch to setup_view
                    auto v = std::make_shared<setup_view>(m_display, m_menu_state, m_inventory);
                    m_menu_state->register_view(v);

                } else if (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    draw_pin_deselect();
                    // Select next pin
                    m_pin_select++;
                    if (m_pin_select > 7) {
                        m_pin_select = 0;
                    }
                    draw_pin_select();

                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    draw_pin_deselect();
                    // Select previous pin
                    if (m_pin_select == 0) {
                        m_pin_select = 7;
                    } else {
                        m_pin_select--;
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
        if (m_pin_select < 4) {
            m_display.drawBitmap1((m_pin_select)*32+m_selection_xcelloffset,
                                m_selection_yoffset, sizeof(arrow_down), 8, arrow_down);
        } else {
            m_display.drawBitmap1((m_pin_select-4)*32+m_selection_xcelloffset,
                                m_selection_yoffset+m_rowoffset, sizeof(arrow_down), 8, arrow_down);
        }
    }

    void over_view::draw_pin_deselect() const {
        // overwrite selection marker with black bitmap
        if (m_pin_select < 4) {
            m_display.drawBitmap1((m_pin_select)*32+m_selection_xcelloffset,
                                m_selection_yoffset, sizeof(black_rect_5x8), 8, black_rect_5x8);
        } else {
            m_display.drawBitmap1((m_pin_select-4)*32+m_selection_xcelloffset,
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

    setup_view::setup_view(DisplaySSD1306_128x64_I2C &d,
                           std::shared_ptr<menu_state> menu_state,
                           std::shared_ptr<inventory> invent)
        : menu_view(d, menu_state, invent)
        , m_menu_items{"Setup port groups",
                       "Go to overview"}
        , m_setup_menu_dimensions{NanoPoint{0, 0}, NanoPoint{127, 63}}
        {
        setup_menu = std::make_unique<LcdGfxMenu>(m_menu_items, 2, m_setup_menu_dimensions);
    }

    setup_view::~setup_view() {
        // nothing to do
    }

    void setup_view::notify(const menu_action &a) {

        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                setup_menu->show(m_display);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    if (setup_menu->selection()) {
                        auto v = std::make_shared<over_view>(m_display, m_menu_state, m_inventory);
                        m_menu_state->register_view(v);
                    } else if (!m_inventory->get_group_dispatcher()->get_port_groups().empty()) {
                        auto gd = m_inventory->get_group_dispatcher();
                        auto v = std::make_shared<portgroup_view>(m_display,
                                                                  m_menu_state,
                                                                  m_inventory,
                                                                  (gd->get_port_groups()).begin());
                        m_menu_state->register_view(v);
                    } else {
                        auto v = std::make_shared<add_portgroup_view>(m_display,
                                                                      m_menu_state,
                                                                      m_inventory);
                        m_menu_state->register_view(v);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // nothing to do
                } else if (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    setup_menu->down();
                    setup_menu->show(m_display);
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    setup_menu->up();
                    setup_menu->show(m_display);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    portgroup_view::portgroup_view(DisplaySSD1306_128x64_I2C &d,
                                   std::shared_ptr<menu_state> menu_state,
                                   std::shared_ptr<inventory> invent,
                                   const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : menu_view(d, menu_state, invent)
        , m_group_dispatcher(*(m_inventory->get_group_dispatcher()))
        , m_cur_group_it(group_it)
        , m_port_group(**m_cur_group_it)
        , m_current_menu_layer(menu_layer::TOP)
        , m_current_pane_selection(menu_pane::TITLE) {
        // nothing to do
    }

    portgroup_view::~portgroup_view() {
        // nothing to do
    }

    void portgroup_view::notify(const menu_action &a) {
        switch (m_current_menu_layer) {
            case menu_layer::TOP :
                notify_top(a);
                break;
            case menu_layer::INOUT_SELECT :
                notify_select(a);
                break;
            default :
                // nothing to do
                break;
        }
    }

    void portgroup_view::notify_top(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                draw_frame();
                draw_title();
                draw_inputs();
                draw_outputs();
                // draw selection marker on title row
                m_display.drawBitmap1(100, 0, sizeof(arrow_down), 8, arrow_down);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    // Show add_portgroup_view if over the last port group
                    if (std::next(m_cur_group_it, 1) == (m_group_dispatcher.get_port_groups()).end()) {
                        auto v = std::make_shared<add_portgroup_view>(m_display,
                                                                      m_menu_state,
                                                                      m_inventory);
                        m_menu_state->register_view(v);
                    } else {
                        // Get iterator to the next port group and create view for it
                        auto v = std::make_shared<portgroup_view>(m_display,
                                                                  m_menu_state,
                                                                  m_inventory,
                                                                  std::next(m_cur_group_it, 1));
                        m_menu_state->register_view(v);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    // Do nothing if at the first port group
                    if (m_cur_group_it == (m_group_dispatcher.get_port_groups()).begin()) {
                        return;
                    }
                    // Get iterator to the previous port group and create view for it
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              std::prev(m_cur_group_it, 1));
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    // Switch to io select layer
                    m_display.drawBitmap1(100, 0, sizeof(black_rect_5x8), 8, black_rect_5x8);
                    m_current_menu_layer = menu_layer::INOUT_SELECT;
                    m_current_pane_selection = menu_pane::INS_PANE;
                    // Trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch to setup_view
                    auto v = std::make_shared<setup_view>(m_display,
                                                          m_menu_state,
                                                          m_inventory);
                    m_menu_state->register_view(v);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    void portgroup_view::notify_select(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                if (m_current_pane_selection == menu_pane::INS_PANE) {
                    m_display.drawBitmap1(56, 56, sizeof(arrow_down), 8, arrow_down);
                } else if (m_current_pane_selection == menu_pane::OUTS_PANE) {
                    m_display.drawBitmap1(120, 56, sizeof(arrow_down), 8, arrow_down);
                }
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    if (m_current_pane_selection == menu_pane::INS_PANE) {
                        m_current_pane_selection = menu_pane::OUTS_PANE;
                        m_display.drawBitmap1(56, 56, sizeof(black_rect_5x8), 8, black_rect_5x8);
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    if (m_current_pane_selection == menu_pane::OUTS_PANE) {
                        m_current_pane_selection = menu_pane::INS_PANE;
                        m_display.drawBitmap1(120, 56, sizeof(black_rect_5x8), 8, black_rect_5x8);
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    // Switch to config_portgroup_view
                    auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                     m_menu_state,
                                                                     m_inventory,
                                                                     m_cur_group_it,
                                                                     m_current_pane_selection);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // overwrite selection marker with black bitmap
                    if (m_current_pane_selection == menu_pane::INS_PANE) {
                        m_display.drawBitmap1(56, 56, sizeof(black_rect_5x8), 8, black_rect_5x8);
                    } else if (m_current_pane_selection == menu_pane::OUTS_PANE) {
                        m_display.drawBitmap1(120, 56, sizeof(black_rect_5x8), 8, black_rect_5x8);
                    }
                    // Switch to top layer
                    m_current_menu_layer = menu_layer::TOP;
                    m_current_pane_selection = menu_pane::TITLE;
                    // Trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    void portgroup_view::draw_frame() const {
        m_display.drawHLine(0, 10, 127);
        m_display.drawVLine(63, 11, 63);
    }

    void portgroup_view::draw_title() const {
        m_display.setFixedFont(ssd1306xled_font6x8);
        m_display.printFixed(5, 0, "Port Group ", STYLE_NORMAL);
        m_display.setTextCursor(70, 0);
        m_display.print(1+(std::distance((m_group_dispatcher.get_port_groups()).begin(), m_cur_group_it)));
        m_display.printFixed(82, 0, "/", STYLE_NORMAL);
        m_display.setTextCursor(88, 0);
        m_display.print(std::distance((m_group_dispatcher.get_port_groups()).begin(),
                                      (m_group_dispatcher.get_port_groups()).end()));
    }

    void portgroup_view::draw_inputs() const {
        m_display.setFixedFont(ssd1306xled_font6x8);
        m_display.printFixed(0, 16, "Ch: ", STYLE_NORMAL);
        m_display.setTextCursor(28, 16);
        m_display.print(m_port_group.get_midi_channel());

        u8 i = 0;
        for (auto &msg_type: m_port_group.get_msg_types()) {
            m_display.printFixed(0, 28+(i*8), midi_msgtype2name(msg_type), STYLE_NORMAL);
            i++;
        }
    }

    void portgroup_view::draw_outputs() const {
        m_display.setFixedFont(ssd1306xled_font6x8);
        m_display.printFixed(66, 16, "Demux: ", STYLE_NORMAL);
        demux_type type = (m_port_group.get_demux()).get_type();
        m_display.printFixed(100, 16, demux_type2name(type), STYLE_NORMAL);

        auto& out_ports = (m_port_group.get_demux()).get_output();
        u8 row = 0, index = 0;
        while (index < out_ports.size()) {
            for (u8 column = 0; column < 4; column++) {
                if (index < out_ports.size()) {
                    m_display.setTextCursor(66+column*12, 28+(row*12));
                    m_display.print((out_ports.at(index)->get_port_number()) + 1);
                    index++;
                } else {
                    break;
                }
            }
            row++;
        }
    }

    config_portgroup_view::config_portgroup_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it,
        const menu_pane io_switch)
        : portgroup_view(d, menu_state, invent, group_it)
        , m_io_switch(io_switch)
        , m_ins_config_menu_items{"Set MIDI channel",
                                  "Add MIDI input",
                                  "Remove MIDI input",
                                  "Delete this portgroup",
                                  "Set Controller",
                                  "Learn MIDI input"}
        , m_outs_config_menu_items{"Set demuxer",
                                   "Add port",
                                   "Remove port",
                                   "Delete this portgroup",
                                   "Set transpose"}
        , m_config_menu_dimensions{NanoPoint{0, 8}, NanoPoint{127, 63}}
        {
        switch (m_io_switch) {
            case menu_pane::INS_PANE :
                config_menu = std::make_unique<LcdGfxMenu>(m_ins_config_menu_items,
                                                           sizeof(m_ins_config_menu_items) / sizeof(char *),
                                                           m_config_menu_dimensions);
                break;
            case menu_pane::OUTS_PANE :
                config_menu = std::make_unique<LcdGfxMenu>(m_outs_config_menu_items,
                                                           sizeof(m_outs_config_menu_items) / sizeof(char *),
                                                           m_config_menu_dimensions);
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_portgroup_view::~config_portgroup_view() {
        // nothing to do
    }

    void config_portgroup_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                if (m_io_switch == menu_pane::INS_PANE) {
                    m_display.printFixed(4, 0, "Configure inputs:", STYLE_NORMAL);
                } else if (m_io_switch == menu_pane::OUTS_PANE) {
                    m_display.printFixed(4, 0, "Configure outputs:", STYLE_NORMAL);
                }
                config_menu->show(m_display);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    config_menu->down();
                    config_menu->show(m_display);
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    config_menu->up();
                    config_menu->show(m_display);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    if ((config_menu->selection() == 0) && (m_io_switch == menu_pane::INS_PANE)) {
                        // Switch to config_portgroup_ch_view
                        auto v = std::make_shared<config_portgroup_ch_view>(m_display,
                                                                            m_menu_state,
                                                                            m_inventory,
                                                                            m_cur_group_it);
                        m_menu_state->register_view(v);
                    } else if ((config_menu->selection() == 0) && (m_io_switch == menu_pane::OUTS_PANE)) {
                        // Switch to config_portgroup_demux_view
                        auto v = std::make_shared<config_portgroup_demux_view>(m_display,
                                                                               m_menu_state,
                                                                               m_inventory,
                                                                               m_cur_group_it);
                        m_menu_state->register_view(v);
                    } else if ((config_menu->selection() == 1) && (m_io_switch == menu_pane::INS_PANE)) {
                        // Switch to config_portgroup_add_msg_view
                        auto v = std::make_shared<config_portgroup_add_msg_view>(m_display,
                                                                                 m_menu_state,
                                                                                 m_inventory,
                                                                                 m_cur_group_it);
                        m_menu_state->register_view(v);
                    } else if ((config_menu->selection() == 1) && (m_io_switch == menu_pane::OUTS_PANE)) {
                        // Switch to config_portgroup_add_port_view
                        auto v = std::make_shared<config_portgroup_add_port_view>(m_display,
                                                                                  m_menu_state,
                                                                                  m_inventory,
                                                                                  m_cur_group_it);
                        m_menu_state->register_view(v);
                    } else if ((config_menu->selection() == 2) && (m_io_switch == menu_pane::INS_PANE)) {
                        // Switch to config_portgroup_rem_msg_view
                        auto v = std::make_shared<config_portgroup_rem_msg_view>(m_display,
                                                                                  m_menu_state,
                                                                                  m_inventory,
                                                                                  m_cur_group_it);
                        m_menu_state->register_view(v);
                    } else if ((config_menu->selection() == 2) && (m_io_switch == menu_pane::OUTS_PANE)) {
                        // Switch to config_portgroup_rem_port_view
                        auto v = std::make_shared<config_portgroup_rem_port_view>(m_display,
                                                                                  m_menu_state,
                                                                                  m_inventory,
                                                                                  m_cur_group_it);
                        m_menu_state->register_view(v);
                    } else if (config_menu->selection() == 3) {
                        // Delete the port group
                        auto gd = m_inventory->get_group_dispatcher();
                        auto pg_id = m_port_group.get_id();
                        gd->remove_port_group(pg_id);
                        // inform inventory about change
                        m_inventory->submit_portgroup_delete(pg_id);
                        // Switch to portgroup_view
                        // check if there is at least 1 port group left
                        if (!gd->get_port_groups().empty()) {
                            auto v = std::make_shared<portgroup_view>(m_display,
                                                                      m_menu_state,
                                                                      m_inventory,
                                                                      (gd->get_port_groups()).begin());
                            m_menu_state->register_view(v);
                        } else {
                            auto v = std::make_shared<add_portgroup_view>(m_display,
                                                                          m_menu_state,
                                                                          m_inventory);
                            m_menu_state->register_view(v);
                        }
                    } else if ((config_menu->selection() == 4) && (m_io_switch == menu_pane::INS_PANE)) {
                        // Switch to config_portgroup_cc_msg_view
                        auto v = std::make_shared<config_portgroup_cc_msg_view>(m_display,
                                                                                m_menu_state,
                                                                                m_inventory,
                                                                                m_cur_group_it);
                        m_menu_state->register_view(v);
                    } else if ((config_menu->selection() == 4) && (m_io_switch == menu_pane::OUTS_PANE)) {
                        // Switch to config_portgroup_transpose_view
                        auto v = std::make_shared<config_portgroup_transpose_view>(m_display,
                                                                                   m_menu_state,
                                                                                   m_inventory,
                                                                                   m_cur_group_it);
                        m_menu_state->register_view(v);
                    } else if ((config_menu->selection() == 5) && (m_io_switch == menu_pane::INS_PANE)) {
                        // Switch to config_portgroup_learn_msg_view
                        auto v = std::make_shared<config_portgroup_learn_msg_view>(m_display,
                                                                                   m_menu_state,
                                                                                   m_inventory,
                                                                                   m_cur_group_it);
                        m_menu_state->register_view(v);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch back to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_portgroup_ch_view::config_portgroup_ch_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : portgroup_view(d, menu_state, invent, group_it)
        , m_channel(1) {
        // nothing to do
    }

    config_portgroup_ch_view::~config_portgroup_ch_view() {
        // nothing to do
    }

    void config_portgroup_ch_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Set MIDI Channel", STYLE_NORMAL);
                m_display.printFixed(4, 24, "Channel:", STYLE_NORMAL);
                m_display.setTextCursor(52, 24);
                m_display.print(m_channel);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    if (m_channel < 16) {
                        m_channel++;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    if (m_channel > 1) {
                        m_channel--;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    m_port_group.set_midi_channel(m_channel);
                    // inform inventory about change
                    m_inventory->submit_portgroup_change(m_port_group.get_id());
                    // Switch back to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch back to config input menu
                    auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                     m_menu_state,
                                                                     m_inventory,
                                                                     m_cur_group_it,
                                                                     menu_pane::INS_PANE);
                    m_menu_state->register_view(v);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_portgroup_demux_view::config_portgroup_demux_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : portgroup_view(d, menu_state, invent, group_it)
        , m_demux(demux_type::FIFO) {
        // nothing to do
    }

    config_portgroup_demux_view::~config_portgroup_demux_view() {
        // nothing to do
    }

    void config_portgroup_demux_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Set Demuxer:", STYLE_NORMAL);
                m_display.printFixed(4, 20, demux_type2name(m_demux), STYLE_NORMAL);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    ++m_demux;
                    // Trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    --m_demux;
                    // Trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    m_port_group.set_demux(m_demux);
                    // inform inventory about change
                    m_inventory->submit_portgroup_change(m_port_group.get_id());
                    // Switch back to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch back to config input menu
                    auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                     m_menu_state,
                                                                     m_inventory,
                                                                     m_cur_group_it,
                                                                     menu_pane::OUTS_PANE);
                    m_menu_state->register_view(v);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_portgroup_add_msg_view::config_portgroup_add_msg_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : portgroup_view(d, menu_state, invent, group_it)
        , k_message_menu_dimensions{NanoPoint{0, 8}, NanoPoint{127, 63}}
        , m_msg_names(midi_message_type_long_names)
        {
        m_message_menu = std::make_unique<LcdGfxMenu>(m_msg_names,
            //FIXME adjust size if midi_message_type_long_names is changed
            8,
            k_message_menu_dimensions);
    }

    config_portgroup_add_msg_view::~config_portgroup_add_msg_view() {
        // nothing to do
    }

    void config_portgroup_add_msg_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Add MIDI Message:", STYLE_NORMAL);
                m_message_menu->show(m_display);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    m_message_menu->down();
                    m_message_menu->show(m_display);
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    m_message_menu->up();
                    m_message_menu->show(m_display);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    u8 sel_msg_code = m_message_menu->selection() + midi_message::message_type::NOTE_OFF;
                    if (sel_msg_code < 0xf) {
                        m_port_group.add_midi_input(static_cast<midi_message::message_type>(sel_msg_code));
                        if (static_cast<midi_message::message_type>(sel_msg_code) == midi_message::message_type::CONTROL_CHANGE) {
                            // Switch to config_portgroup_cc_msg_view
                            auto v = std::make_shared<config_portgroup_cc_msg_view>(m_display,
                                                                                    m_menu_state,
                                                                                    m_inventory,
                                                                                    m_cur_group_it);
                            m_menu_state->register_view(v);
                            return;
                        }
                    } else if (m_message_menu->selection() == 7) {
                        m_port_group.add_midi_input(midi_message::message_type::CLOCK);
                    }
                    // inform inventory about change
                    m_inventory->submit_portgroup_change(m_port_group.get_id());
                    // Switch back to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch back to config input menu
                    auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                     m_menu_state,
                                                                     m_inventory,
                                                                     m_cur_group_it,
                                                                     menu_pane::INS_PANE);
                    m_menu_state->register_view(v);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_portgroup_cc_msg_view::config_portgroup_cc_msg_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : portgroup_view(d, menu_state, invent, group_it)
        , m_cc_number(m_port_group.get_cc()) {
        // nothing to do
    }

    config_portgroup_cc_msg_view::~config_portgroup_cc_msg_view() {
        // nothing to do
    }

    void config_portgroup_cc_msg_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Select a controller:", STYLE_NORMAL);
                m_display.setTextCursor(4, 20);
                m_display.print(m_cc_number);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    m_cc_number++;
                    if (m_cc_number > 127) {
                        m_cc_number = 0;
                    } else if (m_cc_number == 32) {
                        m_cc_number = 64;
                    }
                    // Trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    if (m_cc_number == 64) {
                        m_cc_number = 31;
                    } else if (m_cc_number > 0) {
                        m_cc_number--;
                    } else if (m_cc_number == 0) {
                        m_cc_number = 127;
                    }
                    // Trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    m_port_group.set_cc(m_cc_number);
                    // inform inventory about change
                    m_inventory->submit_portgroup_change(m_port_group.get_id());
                    // Switch to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_portgroup_learn_msg_view::config_portgroup_learn_msg_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : portgroup_view(d, menu_state, invent, group_it)
        , m_control(0)
        , m_capture_msg(midi_message::message_type::NOTE_OFF, 1, 0, 0)
        , m_menu_q(m_inventory->get_menu_queue())
        , m_learn_menu_items{"Add MIDI message",
                             "Recapture",
                             "Cancel"}
        , m_learn_menu_dimensions{NanoPoint{0, 24}, NanoPoint{127, 63}}
        {
            learn_menu = std::make_unique<LcdGfxMenu>(m_learn_menu_items,
                                                      sizeof(m_learn_menu_items) / sizeof(char *),
                                                      m_learn_menu_dimensions);
    }

    config_portgroup_learn_msg_view::~config_portgroup_learn_msg_view() {
        // nothing to do
    }

    void config_portgroup_learn_msg_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                if (m_control == 0) {
                    m_display.printFixed(4, 0, "Learn input:", STYLE_NORMAL);
                    m_display.printFixed(4, 16, "Press button", STYLE_NORMAL);
                    m_display.printFixed(4, 24, "to start capture", STYLE_NORMAL);
                } else if (m_control == 1) {
                    m_display.printFixed(4, 16, "Waiting for input...", STYLE_NORMAL);
                    if (m_group_dispatcher.got_capture()) {
                        m_capture_msg = m_group_dispatcher.get_capture();
                        m_control++;
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    } else {
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_q->add_menu_action(a);
                    }
                } else if (m_control == 2) {
                    m_display.printFixed(4, 0, "Got message:", STYLE_NORMAL);
                    m_display.printFixed(4, 8, midi_msgtype2name(m_capture_msg.type), STYLE_NORMAL);
                    m_display.printFixed(4, 16, "Ch:");
                    m_display.setTextCursor(22, 16);
                    m_display.print(m_capture_msg.channel);
                    if (m_capture_msg.type == midi_message::message_type::CONTROL_CHANGE) {
                        m_display.printFixed(70, 8, "Contr:", STYLE_NORMAL);
                        m_display.setTextCursor(106, 8);
                        m_display.print(m_capture_msg.data0);
                    }
                    learn_menu->show(m_display);
                }
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    m_control++;
                    if (m_control == 1) {
                        auto gd = m_inventory->get_group_dispatcher();
                        gd->activate_capture_mode();
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_q->add_menu_action(a);

                    } else if (m_control == 3 && learn_menu->selection() == 0) {
                        m_port_group.add_midi_input(m_capture_msg.type);
                        m_port_group.set_midi_channel(m_capture_msg.channel);
                        if (m_capture_msg.type == midi_message::message_type::CONTROL_CHANGE) {
                            m_port_group.set_cc(m_capture_msg.data0);
                        }
                        // inform inventory about change
                        m_inventory->submit_portgroup_change(m_port_group.get_id());
                        // Switch back to portgroup_view
                        auto v = std::make_shared<portgroup_view>(m_display,
                                                                  m_menu_state,
                                                                  m_inventory,
                                                                  m_cur_group_it);
                        m_menu_state->register_view(v);

                    } else if (m_control == 3 && learn_menu->selection() == 1) {
                        m_control = 1;
                        auto gd = m_inventory->get_group_dispatcher();
                        gd->activate_capture_mode();
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_q->add_menu_action(a);

                    } else if (m_control == 3 && learn_menu->selection() == 2) {
                        // Switch back to config input menu
                        auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                         m_menu_state,
                                                                         m_inventory,
                                                                         m_cur_group_it,
                                                                         menu_pane::INS_PANE);
                        m_menu_state->register_view(v);
                    }

                } else if (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    if (m_control == 2) {
                        learn_menu->down();
                        learn_menu->show(m_display);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    if (m_control == 2) {
                        learn_menu->up();
                        learn_menu->show(m_display);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch back to config input menu
                    auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                     m_menu_state,
                                                                     m_inventory,
                                                                     m_cur_group_it,
                                                                     menu_pane::INS_PANE);
                    m_menu_state->register_view(v);
                }
                break;
        }
    }

    config_portgroup_rem_msg_view::config_portgroup_rem_msg_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : portgroup_view(d, menu_state, invent, group_it)
        , k_message_menu_dimensions{NanoPoint{0, 8}, NanoPoint{127, 63}}
        , m_msg_types(m_port_group.get_msg_types())
        {
        m_msg_names = new char*[m_msg_types.size()];
        u8 i = 0;
        for (auto &msg_type: m_msg_types) {
            //FIXME this is broken
            strcpy(m_msg_names[i], midi_msgtype2name(msg_type));
            i++;
        }
        m_message_menu = std::make_unique<LcdGfxMenu>(const_cast<const char**>(m_msg_names),
                                                      m_msg_types.size(),
                                                      k_message_menu_dimensions);
    }

    config_portgroup_rem_msg_view::~config_portgroup_rem_msg_view() {
        delete []m_msg_names;
    }

    void config_portgroup_rem_msg_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Select message type to remove:", STYLE_NORMAL);
                m_message_menu->show(m_display);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    m_message_menu->down();
                    m_message_menu->show(m_display);
                    break;
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    m_message_menu->up();
                    m_message_menu->show(m_display);
                    break;
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    m_port_group.remove_msg_type(m_msg_types.at(m_message_menu->selection()));
                    // inform inventory about change
                    m_inventory->submit_portgroup_change(m_port_group.get_id());
                    // Switch back to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch back to config input menu
                    auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                     m_menu_state,
                                                                     m_inventory,
                                                                     m_cur_group_it,
                                                                     menu_pane::INS_PANE);
                    m_menu_state->register_view(v);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_portgroup_add_port_view::config_portgroup_add_port_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : portgroup_view(d, menu_state, invent, group_it)
        , m_port_number(1) {
        // nothing to do
    }

    config_portgroup_add_port_view::~config_portgroup_add_port_view() {
        // nothing to do
    }

    void config_portgroup_add_port_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Add new port to group", STYLE_NORMAL);
                m_display.printFixed(4, 24, "Port:", STYLE_NORMAL);
                m_display.setTextCursor(52, 24);
                m_display.print(m_port_number);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    if (m_port_number < 8) {
                        m_port_number++;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    if (m_port_number > 1) {
                        m_port_number--;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    auto out_port = m_inventory->get_output_port(m_port_number-1);
                    m_port_group.add_port(out_port);
                    // inform inventory about change
                    m_inventory->submit_portgroup_change(m_port_group.get_id());
                    // Switch back to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch back to config output menu
                    auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                     m_menu_state,
                                                                     m_inventory,
                                                                     m_cur_group_it,
                                                                     menu_pane::OUTS_PANE);
                    m_menu_state->register_view(v);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_portgroup_rem_port_view::config_portgroup_rem_port_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : portgroup_view(d, menu_state, invent, group_it)
        , m_port_selection(0) {
        const auto& out_ports = (m_port_group.get_demux()).get_output();
        for (auto &out_port: out_ports) {
            m_port_numbers.push_back(out_port->get_port_number());
        }
    }

    config_portgroup_rem_port_view::~config_portgroup_rem_port_view() {
        // nothing to do
    }

    void config_portgroup_rem_port_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Remove port from group", STYLE_NORMAL);
                m_display.printFixed(4, 24, "Port:", STYLE_NORMAL);
                m_display.setTextCursor(52, 24);
                m_display.print(1 + m_port_numbers.at(m_port_selection));
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    if (m_port_selection < m_port_numbers.size() - 1) {
                        m_port_selection++;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }

                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    if (m_port_selection > 0) {
                        m_port_selection--;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    m_port_group.remove_port(m_port_numbers.at(m_port_selection));
                    // inform inventory about change
                    m_inventory->submit_portgroup_change(m_port_group.get_id());
                    // Switch back to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch back to config output menu
                    auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                     m_menu_state,
                                                                     m_inventory,
                                                                     m_cur_group_it,
                                                                     menu_pane::OUTS_PANE);
                    m_menu_state->register_view(v);
                }
                break;
            default :
                // nothing to do
                break;
        }
    }

    config_portgroup_transpose_view::config_portgroup_transpose_view(
        DisplaySSD1306_128x64_I2C &d,
        std::shared_ptr<menu_state> menu_state,
        std::shared_ptr<inventory> invent,
        const std::vector<std::unique_ptr<port_group>>::const_iterator group_it)
        : portgroup_view(d, menu_state, invent, group_it)
        , m_transpose_offset(m_port_group.get_transpose()) {
        // nothing to do
    }

    config_portgroup_transpose_view::~config_portgroup_transpose_view() {
        // nothing to do
    }

    void config_portgroup_transpose_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.printFixed(4, 0, "Set transpose:", STYLE_NORMAL);
                m_display.setTextCursor(4, 16);
                m_display.print(m_transpose_offset);
                m_display.printFixed(22, 16, "halftones", STYLE_NORMAL);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    m_transpose_offset++;
                    // Trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    m_transpose_offset--;
                    // Trigger display update
                    menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                    m_menu_state->notify(a);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    m_port_group.set_transpose(m_transpose_offset);
                    m_inventory->submit_portgroup_change(m_port_group.get_id());
                    // Switch back to portgroup_view
                    auto v = std::make_shared<portgroup_view>(m_display,
                                                              m_menu_state,
                                                              m_inventory,
                                                              m_cur_group_it);
                    m_menu_state->register_view(v);
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    // Switch back to config output menu
                    auto v = std::make_shared<config_portgroup_view>(m_display,
                                                                     m_menu_state,
                                                                     m_inventory,
                                                                     m_cur_group_it,
                                                                     menu_pane::OUTS_PANE);
                    m_menu_state->register_view(v);
                }
                break;
            default:
                // nothing to do
                break;
        }
    }

    add_portgroup_view::add_portgroup_view(DisplaySSD1306_128x64_I2C &d,
                                           std::shared_ptr<menu_state> menu_state,
                                           std::shared_ptr<inventory> invent)
        : menu_view(d, menu_state, invent)
        , m_group_dispatcher(*(m_inventory->get_group_dispatcher()))
        , m_demux(demux_type::RANDOM)
        , m_channel(1)
        , m_control(0) {
        // nothing to do
    }

    add_portgroup_view::~add_portgroup_view() {
        // nothing to do
    }

    void add_portgroup_view::notify(const menu_action &a) {
        switch (a.m_kind) {
            case menu_action::kind::UPDATE :
                m_display.clear();
                m_display.setFixedFont(ssd1306xled_font6x8);
                m_display.drawHLine(0, 10, 127);
                m_display.printFixed(4, 0, "Add new port group", STYLE_NORMAL);
                m_display.printFixed(8, 16, "Channel: ", STYLE_NORMAL);
                m_display.setTextCursor(56, 16);
                m_display.print(m_channel);
                m_display.printFixed(8, 24, "Demux: ", STYLE_NORMAL);
                m_display.printFixed(56, 24, demux_type2name(m_demux), STYLE_NORMAL);
                draw_selection(m_control);
                break;
            case menu_action::kind::ROT_ACTIVITY :
                if        (a.m_subkind == menu_action::subkind::ROT_RIGHT) {
                    if (m_control == 1 && m_channel < 16) {
                        m_channel++;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    } else if (m_control == 2) {
                        ++m_demux;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_LEFT) {
                    if (m_control == 0) {
                        //switch view to the last portgroup if existent
                        if (!m_group_dispatcher.get_port_groups().empty()) {
                            auto end_it = m_group_dispatcher.get_port_groups().end();
                            auto v = std::make_shared<portgroup_view>(m_display,
                                                                      m_menu_state,
                                                                      m_inventory,
                                                                      std::prev(end_it));
                            m_menu_state->register_view(v);
                        }
                    } else if (m_control == 1 && m_channel > 1) {
                        m_channel--;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    } else if (m_control == 2) {
                        --m_demux;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON) {
                    if (m_control < 2) {
                        m_control++;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    } else {
                        // create new port group and display it
                        m_group_dispatcher.add_port_group(m_demux, m_channel);
                        auto new_pg_it = std::prev(m_group_dispatcher.get_port_groups().end());
                        // inform inventory about change
                        m_inventory->submit_portgroup_add((*new_pg_it)->get_id());
                        auto v = std::make_shared<portgroup_view>(m_display,
                                                                  m_menu_state,
                                                                  m_inventory,
                                                                  new_pg_it);
                        m_menu_state->register_view(v);
                    }
                } else if (a.m_subkind == menu_action::subkind::ROT_BUTTON_LONGPRESS) {
                    if (m_control > 0) {
                        m_control--;
                        // Trigger display update
                        menu_action a(menu_action::kind::UPDATE, menu_action::subkind::NO_SUB);
                        m_menu_state->notify(a);
                    }
                }
                break;
            default:
                // nothing to do
                break;
        }
    }

    void add_portgroup_view::draw_selection(u8 control) {
        switch (control) {
            case 0 :
                // draw selection marker on title row
                m_display.drawBitmap1(120, 0, sizeof(arrow_down), 8, arrow_down);
                break;
            case 1 :
                // draw selection marker on channel row
                m_display.drawBitmap1(0, 16, sizeof(arrow_down), 8, arrow_down);
                break;
            case 2 :
                // draw selection marker on demux row
                m_display.drawBitmap1(0, 24, sizeof(arrow_down), 8, arrow_down);
                break;
            default:
                // nothing to do
                break;
        }
    }

    menu_state::menu_state()
        : menu_interface()
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
