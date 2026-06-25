#ifndef MICRA_E6_MODBUS485_PROTOCOL_H
#define MICRA_E6_MODBUS485_PROTOCOL_H

#include <stdint.h>
#include "plib_modbus485.h"

typedef enum
{
    MICRA_MODBUS_REG_INPUT_RANGE,                       // Input MicraModbusIntput_t, Range MicraModbusRange_t
    MICRA_MODBUS_REG_INPUT1_DIG43,                      // Input 1 digits 4 and 3
    MICRA_MODBUS_REG_INPUT1_DIG21,                      // Input 1 digits 2 and 1
    MICRA_MODBUS_REG_INPUT1_DIG0_INPUT2_DIG4,           // Input 1 digit 0 and Input 2 digit 4
    MICRA_MODBUS_REG_INPUT2_DIG32,                      // Input 2 digits 3 and 2
    MICRA_MODBUS_REG_INPUT2_DIG10,                      // Input 2 digits 1 and 0
    MICRA_MODBUS_REG_DISPLAY1_DIG43,                    // Display 1 digits 4 and 3
    MICRA_MODBUS_REG_DISPLAY1_DIG21,                    // Display 1 digits 2 and 1
    MICRA_MODBUS_REG_DISPLAY1_DIG0_DISPLAY2_DIG4,       // Display 1 digit 0 and Display 2 digit 4
    MICRA_MODBUS_REG_DISPLAY2_DIG32,                    // Display 2 digits 3 and 2
    MICRA_MODBUS_REG_DISPLAY2_DIG10,                    // Display 2 digits 1 and 0
    MICRA_MODBUS_REG_DECIMAL_POINT_FILTER_P,            // Decimal point (from 0 none to 4 digits), Filter (from 0 no filter to 9 strongest)
    MICRA_MODBUS_REG_BRIGHTNESS_ROUND,                  // Britghness (0 high, 1 low), Round (0 no, 1 5 pts, 2 10 pts)
    MICRA_MODBUS_REG_ECO_MINUTES,                       // Minutes eco mode
    MICRA_MODBUS_REG_ECO_MODE_LOGIC1,                   // Eco mode (0 off, 1 on), Logic function 1 (from 0 to 16)
    MICRA_MODBUS_REG_LOGIC23,                           // Logic function 2 and 3 (from 0 to 16)
    MICRA_MODBUS_REG_SETPOINT_FLAG_ARROW_PRINT,         // Setpoint flag (from 0 to 3), Arrow print (0 no, 1 yes)
    MICRA_MODBUS_REG_COLOR,                             // Color PROG (0 red, 1 green, 2 orange), Color RUN (0 red, 1 green, 2 orange)
    MICRA_MODBUS_REG_BLOCK,                             // 
    MICRA_MODBUS_REG_CODE_DIG32,                        // Digits 3 and 2
    MICRA_MODBUS_REG_CODE_DIG10,                        // Digits 1 and 0
    MICRA_MODBUS_REG_SETPOINT12_STATE,                  // Setpoint 1 state (0 off, 1 on), Setpoint 2 state (0 off, 1 on)
    MICRA_MODBUS_REG_SETPOINT34_STATE,                  // Setpoint 3 state (0 off, 1 on), Setpoint 4 state (0 off, 1 on)
    MICRA_MODBUS_REG_SETPOINT12_HI_LO,                  // Setpoint 1 high low (0 high, 1 low), Setpoint 2 high low (0 high, 1 low)
    MICRA_MODBUS_REG_SETPOINT34_HI_LO,                  // Setpoint 3 high low (0 high, 1 low), Setpoint 4 high low (0 high, 1 low)
    MICRA_MODBUS_REG_SETPOINT12_DLY_HYS,                // Setpoint 1 dly hys (0 hys, 1 dly), Setpoint 2 dly hys (0 hys, 1 dly)
    MICRA_MODBUS_REG_SETPOINT23_DLY_HYS,                // Setpoint 3 dly hys (0 hys, 1 dly), Setpoint 4 dly hys (0 hys, 1 dly)
    MICRA_MODBUS_REG_SETPOINT1_DIG43,                   // Setpoint 1 digits 4 and 3
    MICRA_MODBUS_REG_SETPOINT1_DIG21,                   //
    MICRA_MODBUS_REG_SETPOINT1_DIG0_SETPOINT2_DIG4,     //
    MICRA_MODBUS_REG_SETPOINT2_DIG32,                   //
    MICRA_MODBUS_REG_SETPOINT2_DIG10,                   //
    MICRA_MODBUS_REG_SETPOINT3_DIG43,                   //
    MICRA_MODBUS_REG_SETPOINT3_DIG21,                   //
    MICRA_MODBUS_REG_SETPOINT3_DIG0_SETPOINT4_DIG4,     //
    MICRA_MODBUS_REG_SETPOINT4_DIG32,
    MICRA_MODBUS_REG_SETPOINT4_DIG10,
    MICRA_MODBUS_REG_DLY_HYS_SET1_DIG43,
    MICRA_MODBUS_REG_DLY_HYS_SET1_DIG21,
    MICRA_MODBUS_REG_DLY_HYS_SET1_DIG0_SET2_DIG4,
    MICRA_MODBUS_REG_DLY_HYS_SET2_DIG32,
    MICRA_MODBUS_REG_DLY_HYS_SET2_DIG10,
    MICRA_MODBUS_REG_DLY_HYS_SET3_DIG43,
    MICRA_MODBUS_REG_DLY_HYS_SET3_DIG21,
    MICRA_MODBUS_REG_DLY_HYS_SET3_DIG0_SET4_DIG4,
    MICRA_MODBUS_REG_DLY_HYS_SET4_DIG32,
    MICRA_MODBUS_REG_DLY_HYS_SET4_DIG10,
    
    MICRA_MODBUS_REG_SETPOINT12_COLOR,
    MICRA_MODBUS_REG_SETPOINT34_COLOR,
    MICRA_MODBUS_REG_ANALOG_HIGH_DIGIT43,
    MICRA_MODBUS_REG_ANALOG_HIGH_DIGIT21,
    MICRA_MODBUS_REG_ANALOG_HIGH_DIGIT0_LOW_DIGIT4,
    MICRA_MODBUS_REG_ANALOG_LOW_DIGIT32,
    MICRA_MODBUS_REG_ANALOG_DIGIT10,
    MICRA_MODBUS_REG_54_RESERVED,
    MICRA_MODBUS_REG_55_RESERVED,
    MICRA_MODBUS_REG_56_RESERVED,
    MICRA_MODBUS_REG_57_RESERVED,
    MICRA_MODBUS_REG_58_RESERVED,
    MICRA_MODBUS_REG_BAUDRATE,                               // First unused, baud rate
    MICRA_MODBUS_REG_RS_DIRECTION_DIGIT10,
    MICRA_MODBUS_REG_RS_PROTOCOL_DELAY,                      // Protocol (0 ASCII, 1 ISO 1745, 2 Modbus), Delay (0 30 ms, 1 60 ms, 2 100 ms)

    // only read variables
    MICRA_MODBUS_REG_DISPLAY,
    MICRA_MODBUS_REG_SETPOINT1_VALUE,
    MICRA_MODBUS_REG_SETPOINT2_VALUE,
    MICRA_MODBUS_REG_SETPOINT3_VALUE,
    MICRA_MODBUS_REG_SETPOINT4_VALUE,
    MICRA_MODBUS_REG_POSITIVE_PEAK_VALUE,
    MICRA_MODBUS_REG_NEGATIVE_PEAK_VALUE,
    MICRA_MODBUS_REG_RELAY12_STATE,
    MICRA_MODBUS_REG_RELAY34_STATE,
    MICRA_MODBUS_REG_ANALOG_OUTPUT_HIGH,
    MICRA_MODBUS_REG_ANALOG_OUTPUT_LOW,
    MICRA_MODBUS_REG_SIGNEDNESS,
    MICRA_MODBUS_REG_SOFTWARE_VERSION
}MicraE6ModbusReg_t;

typedef enum
{
    MICRA_INPUT_DC_VOLT,
    MICRA_INPUT_AC_VOLT,
    MICRA_INPUT_DC_AMP,
    MICRA_INPUT_AC_AMP
}MicraE6Input_t;

typedef enum
{
    MICRA_RANGE_600V,
    MICRA_RANGE_200V,
    MICRA_RANGE_20V,
    MICRA_RANGE_2V
}MicraE6VoltRange_t;

typedef enum
{
    MICRA_RANGE_5A,
    MICRA_RANGE_1A,
    MICRA_RANGE_200mA,
    MICRA_RANGE_SHUNT_100mV,
    MICRA_RANGE_SHUNT_60mV,
    MICRA_RANGE_SHUNT_50mV
}MicraE6AmpRange_t;

// Public API

#ifdef __cplusplus
extern "C" {
#endif

uint8_t micra_modbus485_generate_read_packet(Modbus485Comm_t* comm, uint16_t first_word, uint16_t nb_words, uint8_t* out);
uint8_t micra_modbus485_generate_write_packet(Modbus485Comm_t* comm, uint16_t first_word,uint16_t nb_words, uint8_t* data, uint8_t* out);
uint8_t micra_modbus485_parse_response(Modbus485Comm_t* comm, uint8_t* rx, uint8_t rx_len);

#ifdef __cplusplus
}
#endif

#endif  // MICRA_E6_MODBUS485_PROTOCOL_H