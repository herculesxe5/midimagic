#ifndef MIDIMAGIC_OUTPUT_H
#define MIDIMAGIC_OUTPUT_H

#include "common.h"
#include "menu.h"
#include <vector>
#include <queue>
#include <memory>

namespace midimagic {
    class midi_message;
    class ad57x4;

    class output_port {
    public:
        explicit output_port(u8 digital_pin, u8 dac_channel, ad57x4 &dac,
                             std::shared_ptr<menu_state> menu, u8 port_number);
        output_port() = delete;

        bool is_active();
        bool is_note(midi_message &msg);
        void set_note(midi_message &note_on_msg);
        u8   get_note();
        void end_note();

    private:
        u8 m_digital_pin;
        u8 m_dac_channel;
        ad57x4 &m_dac;
        u8 m_current_note;
        std::shared_ptr<menu_state> m_menu;
        u8 m_port_number;
        const menu_action::kind m_menu_action_kind;
    };

    class output_demux {
    public:
        output_demux();
        output_demux(const output_demux&) = delete;
        virtual ~output_demux();

        virtual void add_note(midi_message& msg) = 0;

        virtual void add_output(output_port p);
        virtual void remove_note(midi_message& msg);
    protected:
        bool set_note(midi_message &msg);
        std::vector<output_port> m_ports;
        std::vector<midi_message> m_msgs;
    };

    class random_output_demux : public output_demux {
    public:
        random_output_demux();
        random_output_demux(const random_output_demux&) = delete;
        virtual ~random_output_demux();

        void add_note(midi_message& msg) override;
    };

    class identic_output_demux : public output_demux {
    public:
        identic_output_demux();
        identic_output_demux(const identic_output_demux&) = delete;
        virtual ~identic_output_demux();
        void add_note(midi_message& msg) override;
    };

    class fifo_output_demux : public output_demux {
        fifo_output_demux();
        fifo_output_demux(const fifo_output_demux&) = delete;
        virtual ~fifo_output_demux();
        void add_note(midi_message& msg) override;
    };
}

#endif //MIDIMAGIC_OUTPUT_H
