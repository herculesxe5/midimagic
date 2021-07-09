#ifndef MIDIMAGIC_MENU_H
#define MIDIMAGIC_MENU_H

#include <memory>

#include <lcdgfx.h>
#include <lcdgfx_gui.h>

#include "common.h"
#include "bitmaps.h"

namespace midimagic {

    struct menu_action {

        enum kind {
            UPDATE,
            PORT_ACTIVITY,
            ROT_ACTIVITY,
        };

        enum subkind {
            NO_SUB,
            ROT_LEFT,
            ROT_RIGHT,
            ROT_BUTTON,
            ROT_BUTTON_LONGPRESS,
            PORT_ACTIVE,
            PORT_NACTIVE,
        };
        explicit menu_action(kind k, subkind sk, int d0 = 0, int d1 = 0);
        menu_action() = delete;
        menu_action(const menu_action&) = delete;
        ~menu_action();
        kind m_kind;
        subkind m_subkind;
        int m_data0;
        int m_data1;

    };

    class menu_state;

    class menu_view {
    public:
        explicit menu_view(DisplaySSD1306_128x64_I2C &d, std::shared_ptr<menu_state> menu_state);
        menu_view() = delete;
        menu_view(const menu_view&) = delete;
        virtual ~menu_view();

        virtual void notify(const menu_action &a) = 0;

    protected:
        DisplaySSD1306_128x64_I2C &m_display;
        std::shared_ptr<menu_state> m_menu_state;
    };

    class pin_view : public menu_view {
    public:
        explicit pin_view(u8 pin, DisplaySSD1306_128x64_I2C &d, std::shared_ptr<menu_state> menu_state);
        pin_view() = delete;
        pin_view(const pin_view&) = delete;
        virtual ~pin_view();

        virtual void notify(const menu_action &a) override;

    private:
        u8 m_pin;
        const char *m_menu_items[6];
        const NanoRect m_pin_menu_dimensions;
        std::shared_ptr<LcdGfxMenu> pin_menu;
    };

    class over_view : public menu_view {
    public:
        over_view(DisplaySSD1306_128x64_I2C &d, std::shared_ptr<menu_state> menu_state);
        over_view(const over_view&) = delete;
        virtual ~over_view();

        virtual void notify(const menu_action &a) override;

    private:
        u8 m_pin_select;

        // Pixel offsets for pin selection and activity
        const uint8_t m_rowoffset;
        const uint8_t m_activitydot_xoffset;
        const uint8_t m_port_value_xoffset;
        const uint8_t m_activity_yoffset;
        const uint8_t m_selection_xcelloffset;
        const uint8_t m_selection_yoffset;

        void draw_statics() const;
        void draw_pin_select() const;
        void draw_pin_deselect() const;
        void draw_activity(const int port_number, const int port_value) const;
        void draw_inactivity(const int port_number) const;
    };

    class menu_state {
    public:
        menu_state();
        menu_state(const menu_state&) = delete;

        void register_view(std::shared_ptr<menu_view> m);
        void notify(const menu_action &a);

    private:
        std::shared_ptr<menu_view> m_view;
    };
}

#endif //MIDIMAGIC_MENU_H
