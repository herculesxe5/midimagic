#ifndef MIDIMAGIC_OUTPUT_H
#define MIDIMAGIC_OUTPUT_H

#include "common.h"
#include "menu_action_queue.h"
#include <vector>
#include <queue>
#include <memory>

namespace midimagic {
    enum demux_type {
        RANDOM = 0,
        IDENTIC,
        FIFO
    };

    class midi_message;
    class ad57x4;

    demux_type& operator++(demux_type& dt);
    demux_type& operator--(demux_type& dt);
    const char* demux_type2name(demux_type type);

    class output_port {
    public:
        explicit output_port(u8 digital_pin, u8 dac_channel, ad57x4 &dac,
                             std::shared_ptr<menu_action_queue> menu, u8 port_number);
        output_port() = delete;

        bool is_active();
        bool is_note(midi_message &msg);
        void set_note(midi_message &note_on_msg);
        u8   get_note();
        void end_note();
        const u8 get_digital_pin() const;
        const u8 get_port_number() const;

    private:
        u8 m_digital_pin;
        u8 m_dac_channel;
        ad57x4 &m_dac;
        u8 m_current_note;
        std::shared_ptr<menu_action_queue> m_menu;
        u8 m_port_number;
    };

    class output_demux {
    public:
        output_demux(const demux_type type);
        output_demux(const output_demux&) = delete;
        virtual ~output_demux();

        virtual void add_note(midi_message& msg) = 0;

        virtual void add_output(output_port p);
        virtual void remove_output(u8 digital_pin);
        virtual void remove_note(midi_message& msg);
        const std::vector<std::unique_ptr<output_port>>& get_output() const;
        const demux_type get_type() const;
    protected:
        bool set_note(midi_message &msg);
        std::vector<std::unique_ptr<output_port>> m_ports;
        std::vector<midi_message> m_msgs;
        const demux_type m_type;
    };

    class random_output_demux : public output_demux {
    public:
        random_output_demux(const demux_type type);
        random_output_demux(const random_output_demux&) = delete;
        virtual ~random_output_demux();

        void add_note(midi_message& msg) override;
    };

    class identic_output_demux : public output_demux {
    public:
        identic_output_demux(const demux_type type);
        identic_output_demux(const identic_output_demux&) = delete;
        virtual ~identic_output_demux();
        void add_note(midi_message& msg) override;
    };

    class fifo_output_demux : public output_demux {
    public:
        fifo_output_demux(const demux_type type);
        fifo_output_demux(const fifo_output_demux&) = delete;
        virtual ~fifo_output_demux();
        void add_note(midi_message& msg) override;
    };
}

#endif //MIDIMAGIC_OUTPUT_H
