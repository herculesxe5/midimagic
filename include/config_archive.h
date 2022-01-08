#ifndef MIDIMAGIC_CONFIG_ARCHIVE_H
#define MIDIMAGIC_CONFIG_ARCHIVE_H

#include "common.h"
#include "system_config.h"
#include "utility/stm32_eeprom.h"
#include "midi_types.h"
#include "output.h"

namespace midimagic {
    class config_archive {
    public:
        config_archive();
        config_archive(const struct system_config system_state);
        ~config_archive();
        config_archive(const config_archive&) = delete;

        enum operation_result {
            SUCCESS = 0,
            NO_ARCHIVE_FOUND,
            VERSION_UNKNOWN,
            SIZE_MISMATCH,
            ARCHIVE_EMPTY,
            CORRUPT_HEADER,
            ILLEGAL_ADDRESS_ON_WRITE,
            ILLEGAL_ADDRESS_ON_READ,
            ILLEGAL_CONFIG_BASE_ADDRESS,
            UNKNOWN_CONFIG_TYPE_ON_READ
        };

        // copy external system_state into class member
        void readin(const struct system_config system_state);
        // return current saved system state
        const struct system_config spellout() const;
        // load eeprom buffer from flash and deserialise into system_state struct
        const operation_result loadon();
        // serialise system_state member into eeprom buffer and write to flash
        const operation_result writeout();

    private:
        #define RUNNING_VERSION 1
        #define MAGIC 0x4d4d // "MM"

        enum static_header_field : u_int32_t {
            MAGIC0 = 0,
            MAGIC1,
            VERSION,
            SIZE0,
            SIZE1,
            PORT_CONFIG_COUNT,
            PORTGROUP_CONFIG_COUNT,
            FIRST_CONFIG_BASE_ADDR
        };

        enum v1_port_config_field : u_int32_t {
            PORT_NUMBER = 0,
            CLOCK_RATE,
            VELOCITY
        };

        enum v1_portgroup_config_field : u_int32_t {
            DEMUX_TYPE = 0,
            MIDI_CHANNEL,
            CC_NUMBER,
            TRANSPOSE,
            INPUT_TYPE_COUNT,
            OUTPUT_PORT_COUNT,
            FIRST_VARIABLE
        };

        enum config_type {
            OUTPUT_PORT_CONFIG = 0,
            PORTGROUP_CONFIG
        };

        // generate header in the eeproom buffer from system_state, return size
        uint32_t generate_archive_header();
        // read header to return base address of configuration at index, return 0 on failure
        uint32_t get_address_to(config_type type, u8 index);
        // serialise config struct and write to eeprom buffer, return size
        uint32_t serialise(struct output_port_config config, uint32_t base_addr);
        uint32_t serialise(struct port_group_config config, uint32_t base_addr);
        // deserialise struct of type from eeprom buffer, write to system_state
        const operation_result deserialise(config_type type, uint32_t base_addr);
        void write_to_flash();
        void load_from_flash();

        struct system_config m_system_state;
        const uint32_t k_port_config_size = 3;
        const uint32_t k_fixed_portgroup_config_size = 6;
        u8 m_running_portgroup_id;
    };
} // namespace midimagic
#endif // MIDIMAGIC_CONFIG_ARCHIVE_H