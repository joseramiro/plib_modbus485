#ifndef PCE_MODBUS_ASCII_PROTOCOL_H
#define PCE_MODBUS_ASCII_PROTOCOL_H

#include <stdint.h>
#include "plib_modbus485.h"


typedef enum
{
    PCE_REGISTER_DISPLAY1,      // Display 1 value
    PCE_REGISTER_MAXMEM,        // Memory of maximum
    PCE_REGISTER_MINMEN,        // Memory of minimum
    PCE_REGISTER_AL1,           // Setpoint 1 value
    PCE_REGISTER_AL2,           // Setpoint 2 value
    PCE_REGISTER_AL3,           // Setpoint 3 value
    PCE_REGISTER_STATUS,        // Alarm status
}PceAsciiRegisters_t;

// Public API


#ifdef __cplusplus
extern "C" {
#endif

uint8_t pce_modbus_ascii_generate_read_packet(Modbus485Comm_t* comm, uint16_t reg, uint16_t len, uint8_t* out);
uint8_t pce_modbus_ascii_parse_response(Modbus485Comm_t* comm, uint8_t* rx, uint8_t rx_len);

#ifdef __cplusplus
}
#endif

#endif  // PCE_MODBUS_ASCII_PROTOCOL_H