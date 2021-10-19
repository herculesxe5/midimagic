#ifndef MIDIMAGIC_MENU_H
#define MIDIMAGIC_MENU_H

#include <memory>

#include <lcdgfx.h>
#include <lcdgfx_gui.h>

#include "common.h"
#include "bitmaps.h"
#include "port_group.h"
#include "inventory.h"
#include "menu_interface.h"

namespace midimagic {
    class menu_state;

    class menu_view {
    public:
        explicit menu_view(DisplaySSD1306_128x64_I2C &d,
                           std::shared_ptr<menu_state> menu_state,
                           std::shared_ptr<inventory> invent);
        menu_view() = delete;
        menu_view(const menu_view&) = delete;
        virtual ~menu_view();

        virtual void notify(const menu_action &a) = 0;

    protected:
        DisplaySSD1306_128x64_I2C &m_display;
        std::shared_ptr<menu_state> m_menu_state;
        std::shared_ptr<inventory> m_inventory;
    };

    class port_view : public menu_view {
    public:
        explicit port_view(u8 port_number,
                          DisplaySSD1306_128x64_I2C &d,
                          std::shared_ptr<menu_state> menu_state,
                          std::shared_ptr<inventory> invent);
        port_view() = delete;
        port_view(const port_view&) = delete;
        virtual ~port_view();

        virtual void notify(const menu_action &a) override;

    protected:
        const u8 m_port_number;
        std::shared_ptr<output_port> m_port;

    private:
        const char *m_menu_items[3];
        const NanoRect m_port_menu_dimensions;
        std::unique_ptr<LcdGfxMenu> m_port_menu;
    };

    class config_port_clock_view : public port_view {
    public:
        explicit config_port_clock_view(u8 port_number,
                                        DisplaySSD1306_128x64_I2C &d,
                                        std::shared_ptr<menu_state> menu_state,
                                        std::shared_ptr<inventory> invent);
        config_port_clock_view() = delete;
        config_port_clock_view(const config_port_clock_view&) = delete;
        virtual ~config_port_clock_view();

        virtual void notify(const menu_action &a) override;

    private:
        u8 m_clock_rate;
    };

    class over_view : public menu_view {
    public:
        over_view(DisplaySSD1306_128x64_I2C &d,
                  std::shared_ptr<menu_state> menu_state,
                  std::shared_ptr<inventory> invent);
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

    class setup_view : public menu_view {
    public:
        setup_view(DisplaySSD1306_128x64_I2C &d,
                   std::shared_ptr<menu_state> menu_state,
                   std::shared_ptr<inventory> invent);
        setup_view(const setup_view&) = delete;
        virtual ~setup_view();

        virtual void notify(const menu_action &a) override;

    private:
        const char *m_menu_items[2];
        const NanoRect m_setup_menu_dimensions;
        std::unique_ptr<LcdGfxMenu> setup_menu;
    };

    class portgroup_view : public menu_view {
    public:
        enum menu_layer {
            TOP = 0,
            INOUT_SELECT,
            INOUT_CONFIG
        };
        enum menu_pane {
            TITLE = 0,
            INS_PANE,
            OUTS_PANE
        };

        portgroup_view(DisplaySSD1306_128x64_I2C &d,
                       std::shared_ptr<menu_state> menu_state,
                       std::shared_ptr<inventory> invent,
                       const std::vector<std::unique_ptr<port_group>>::const_iterator group_it);
        portgroup_view(const portgroup_view&) = delete;
        virtual ~portgroup_view();

        virtual void notify(const menu_action &a) override;

    protected:
        const group_dispatcher& m_group_dispatcher;
        const std::vector<std::unique_ptr<port_group>>::const_iterator m_cur_group_it;
        port_group& m_port_group;
        menu_pane m_current_pane_selection;

    private:
        menu_layer m_current_menu_layer;
        void notify_top(const menu_action &a);
        void notify_select(const menu_action &a);

        void draw_frame() const;
        void draw_title() const;
        void draw_inputs() const;
        void draw_outputs() const;
        void draw_selection() const;
    };

    class config_portgroup_view : public portgroup_view {
    public:
        config_portgroup_view(DisplaySSD1306_128x64_I2C &d,
                              std::shared_ptr<menu_state> menu_state,
                              std::shared_ptr<inventory> invent,
                              const std::vector<std::unique_ptr<port_group>>::const_iterator group_it,
                              const menu_pane io_switch);
        config_portgroup_view(const config_portgroup_view&) = delete;
        virtual ~config_portgroup_view();

        virtual void notify(const menu_action &a) override;
    private:
        const menu_pane m_io_switch;
        const char *m_ins_config_menu_items[4];
        const char *m_outs_config_menu_items[4];
        const NanoRect m_config_menu_dimensions;
        std::unique_ptr<LcdGfxMenu> config_menu;
    };

    class config_portgroup_ch_view : public portgroup_view {
    public:
        config_portgroup_ch_view(DisplaySSD1306_128x64_I2C &d,
                                 std::shared_ptr<menu_state> menu_state,
                                 std::shared_ptr<inventory> invent,
                                 const std::vector<std::unique_ptr<port_group>>::const_iterator group_it);
        config_portgroup_ch_view(const config_portgroup_ch_view&) = delete;
        virtual ~config_portgroup_ch_view();

        virtual void notify(const menu_action &a) override;
    private:
        u8 m_channel;
    };

    class config_portgroup_demux_view : public portgroup_view {
    public:
        config_portgroup_demux_view(DisplaySSD1306_128x64_I2C &d,
                                 std::shared_ptr<menu_state> menu_state,
                                 std::shared_ptr<inventory> invent,
                                 const std::vector<std::unique_ptr<port_group>>::const_iterator group_it);
        config_portgroup_demux_view(const config_portgroup_demux_view&) = delete;
        virtual ~config_portgroup_demux_view();

        virtual void notify(const menu_action &a) override;
    private:
        demux_type m_demux;
    };

    class config_portgroup_add_msg_view : public portgroup_view {
    public:
        config_portgroup_add_msg_view(DisplaySSD1306_128x64_I2C &d,
                                      std::shared_ptr<menu_state> menu_state,
                                      std::shared_ptr<inventory> invent,
                                      const std::vector<std::unique_ptr<port_group>>::const_iterator group_it);
        config_portgroup_add_msg_view(const config_portgroup_add_msg_view&) = delete;
        virtual ~config_portgroup_add_msg_view();

        virtual void notify(const menu_action &a) override;
    private:
        const char** m_msg_names;
        const NanoRect k_message_menu_dimensions;
        std::unique_ptr<LcdGfxMenu> m_message_menu;
    };

    class config_portgroup_rem_msg_view : public portgroup_view {
    public:
        config_portgroup_rem_msg_view(DisplaySSD1306_128x64_I2C &d,
                                      std::shared_ptr<menu_state> menu_state,
                                      std::shared_ptr<inventory> invent,
                                      const std::vector<std::unique_ptr<port_group>>::const_iterator group_it);
        config_portgroup_rem_msg_view(const config_portgroup_rem_msg_view&) = delete;
        virtual ~config_portgroup_rem_msg_view();

        virtual void notify(const menu_action &a) override;
    private:
        const std::vector<midi_message::message_type>& m_msg_types;
        char** m_msg_names;
        const NanoRect k_message_menu_dimensions;
        std::unique_ptr<LcdGfxMenu> m_message_menu;
    };

    class config_portgroup_add_port_view : public portgroup_view {
    public:
        config_portgroup_add_port_view(DisplaySSD1306_128x64_I2C &d,
                                      std::shared_ptr<menu_state> menu_state,
                                      std::shared_ptr<inventory> invent,
                                      const std::vector<std::unique_ptr<port_group>>::const_iterator group_it);
        config_portgroup_add_port_view(const config_portgroup_add_port_view&) = delete;
        virtual ~config_portgroup_add_port_view();

        virtual void notify(const menu_action &a) override;
    private:
        u8 m_port_number;
    };

    class config_portgroup_rem_port_view : public portgroup_view {
    public:
        config_portgroup_rem_port_view(DisplaySSD1306_128x64_I2C &d,
                                      std::shared_ptr<menu_state> menu_state,
                                      std::shared_ptr<inventory> invent,
                                      const std::vector<std::unique_ptr<port_group>>::const_iterator group_it);
        config_portgroup_rem_port_view(const config_portgroup_rem_port_view&) = delete;
        virtual ~config_portgroup_rem_port_view();

        virtual void notify(const menu_action &a) override;
    private:
        std::vector<u8> m_port_numbers;
        u8 m_port_selection;
    };

    class add_portgroup_view : public menu_view {
    public:
        add_portgroup_view(DisplaySSD1306_128x64_I2C &d,
                           std::shared_ptr<menu_state> menu_state,
                           std::shared_ptr<inventory> invent);
        add_portgroup_view(const add_portgroup_view&) = delete;
        virtual ~add_portgroup_view();

        virtual void notify(const menu_action &a) override;
    private:
        group_dispatcher& m_group_dispatcher;
        demux_type m_demux;
        u8 m_channel;
        u8 m_control;

        void draw_selection(u8 control);
    };

    class menu_state : public menu_interface {
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
