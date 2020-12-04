#ifndef MIDIMAGIC_MENU_H
#define MIDIMAGIC_MENU_H

#include <memory>

#include <Adafruit_SSD1306.h>

#include "common.h"

namespace midimagic {
    enum menu_action {
        MEN_INIT,
        MEN_LEFT,
        MEN_RIGHT,
        MEN_ENTER
    };

    class menu_state;

    class menu_view {
    public:
        explicit menu_view(menu_state &s, Adafruit_SSD1306 &d);
        menu_view() = delete;
        menu_view(const menu_view&) = delete;
        virtual ~menu_view();

        virtual void notify() const;

    protected:
        menu_state &m_state;
        Adafruit_SSD1306 &m_display;
    };

    class pin_view : public menu_view {
    public:
        explicit pin_view(u8 pin, menu_state &s, Adafruit_SSD1306 &d);
        pin_view() = delete;
        pin_view(const pin_view&) = delete;
        virtual ~pin_view();

        virtual void notify() const override;

    private:
        u8 m_pin;
    };

    class over_view : public menu_view {
    public:
        over_view(menu_state& s, Adafruit_SSD1306 &d);
        over_view(const over_view&) = delete;
        virtual ~over_view();

        virtual void notify() const override;
    };

    class menu_state {
    public:
        enum rot_action {
            ROT_LEFT,
            ROT_RIGHT,
            ROT_BUTTON
        };

        menu_state();
        menu_state(const menu_state&) = delete;

        void register_view(std::shared_ptr<menu_view> m);
        void notify(const rot_action a);
        // FIXME remove the count demo!!!
        int m_count;
    private:
        std::shared_ptr<menu_view> m_view;
    };
}

#endif //MIDIMAGIC_MENU_H
