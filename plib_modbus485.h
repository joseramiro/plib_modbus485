#ifndef PLIB_MODBUS485_H
#define PLIB_MODBUS485_H

#include <stdint.h>

typedef enum
{
    MODBUS485_MODE_READ_WORD = 3,
    MODBUS485_MODE_FORCE_STATE = 5,
    MODBUS485_MODE_WRITE_WORD = 16,
    MODBUS485_MODE_ERROR = 0x80
}Modbus485Modes_t;

struct Modbus485Comm_t;

typedef struct
{
    uint8_t(*Read_Words)(struct Modbus485Comm_t*, uint16_t, uint16_t, uint8_t*);
    uint8_t(*Write_Words)(struct Modbus485Comm_t*, uint16_t, uint16_t, uint8_t*, uint8_t*);
    uint8_t(*Parse_Response)(struct Modbus485Comm_t*, uint8_t*, uint8_t);
}Modbus485Parser_t;


typedef struct Modbus485Comm_t
{
    uint8_t address;
    uint16_t last_word;
    uint8_t response_received;
    uint8_t last_status;
    uint16_t timeout;
    uint16_t timeout_occured;
    uint8_t error;
    Modbus485Parser_t parser;
    int16_t measure;
}Modbus485Comm_t;

typedef enum
{
    MODBUS_ERR_BAD_LEN,
    MODBUS_ERR_BAD_SLAVE_ADDRESS,
    MODBUS_ERR_RESPONSE,
    MODBUS_ERR_BAD_CRC_ERROR,
    MODBUS_ERR_BAD_CRC_WRITE,
    MODBUS_ERR_BAD_CRC_READ,
    MODBUS_ERR_OTHER,
    MODBUS_OK
}ModbusParseStatus_t;

#endif  // PLIB_MODBUS485_H