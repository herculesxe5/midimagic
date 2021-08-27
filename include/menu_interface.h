#ifndef MIDIMAGIC_MENU_INTERFACE_H
#define MIDIMAGIC_MENU_INTERFACE_H

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
        //menu_action(const menu_action&) = delete;
        ~menu_action();
        kind m_kind;
        subkind m_subkind;
        int m_data0;
        int m_data1;
    };

    class menu_interface {
    public:
        menu_interface() {};
        menu_interface(const menu_interface&) = delete;
        virtual ~menu_interface() {};

        virtual void notify(const menu_action &a) = 0;
    };
}

#endif //MIDIMAGIC_MENU_INTERFACE_H