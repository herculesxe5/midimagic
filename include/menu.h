#ifndef MIDIMAGIC_MENU_H
#define MIDIMAGIC_MENU_H

#include <memory>

#include <Adafruit_SSD1306.h>

#include "common.h"

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

    class menu_view {
    public:
        explicit menu_view(Adafruit_SSD1306 &d);
        menu_view() = delete;
        menu_view(const menu_view&) = delete;
        virtual ~menu_view();

        virtual void notify(const menu_action &a) const = 0;

    protected:
        Adafruit_SSD1306 &m_display;
    };

    class pin_view : public menu_view {
    public:
        explicit pin_view(u8 pin, Adafruit_SSD1306 &d);
        pin_view() = delete;
        pin_view(const pin_view&) = delete;
        virtual ~pin_view();

        virtual void notify(const menu_action &a) const override;

    private:
        u8 m_pin;
    };

    class over_view : public menu_view {
    public:
        over_view(Adafruit_SSD1306 &d);
        over_view(const over_view&) = delete;
        virtual ~over_view();

        virtual void notify(const menu_action &a) const override;

    private:
        // Pixel offsets from cell border for port activity
        const uint8_t m_rowoffset;
        const uint8_t m_activitydot_xoffset;
        const uint8_t m_port_value_xoffset;
        const uint8_t m_activity_yoffset;

        void draw_statics() const;
        void draw_activity(const int port_number, const int port_value) const;
        void draw_inactivity(const int port_number) const;
    };

    class menu_state {
    public:

        menu_state();
        menu_state(const menu_state&) = delete;

        void register_view(std::shared_ptr<menu_view> m);
        void notify(const menu_action &a);
        // FIXME Test debouncing
        //Rotary encoder ISR timestamp
        volatile unsigned long m_rot_int_ts;
        //Rotary encoder threshold, ms
        const unsigned int k_rot_int_th = 10;

    private:
        std::shared_ptr<menu_view> m_view;
    };
}

#endif //MIDIMAGIC_MENU_H
