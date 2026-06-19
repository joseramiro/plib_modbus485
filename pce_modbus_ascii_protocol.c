#include "pce_modbus_ascii_protocol.h"

typedef enum
{
    PCE_POSITION_HEADER_START,
    PCE_POSITION_HEADER_ID,
    PCE_POSITION_HEADER_RSV0,
    PCE_POSITION_HEADER_FROM,
    PCE_POSITION_HEADER_TO,
    PCE_POSITION_HEADER_REG,
    PCE_POSITION_HEADER_RSV1,
    PCE_POSITION_HEADER_LONG
}PceAsciiPositions_t;

typedef enum
{
    PCE_FRAME_PING = 32,
    PCE_FRAME_PONG = 33,
    PCE_FRAME_READ = 36,
    PCE_FRAME_ANS = 37,
    PCE_FRAME_ERR = 38
}PceAsciiFrames_t;

typedef enum
{
    PCE_ERROR_CODE_UNKNOWN_REG = 1,
    PCE_ERROR_CODE_DISPLAY_OVERRANGE,
    PCE_ERROR_CODE_DISPLAY_UNDERRANGE,
    PCE_ERROR_CODE_CRC_ERROR,
    PCE_ERROR_CODE_INTERNAL_ERROR
}PceAsciiErrorCodes_t;

// Display1 register: contains the display value in ASCII incluiding polarity
    // ex. + 0 6 5 4 3 . 2 = 6543.2
    // ex. - 0 0 0 4 . 5 2 = -4.52

#define PCE_PROTOCOL_START          2
#define PCE_PROTOCOL_RSV            32
#define PCE_PROTOCOL_MASTER_ADDR    0
#define PCE_PROTOCOL_END            3

#define PCE_PROTOCOL_CONV           32  // to add to convert real value to coded value

// Static functions

static uint8_t calculate_crc(unsigned char* frame, int8_t len);
static uint8_t check_crc(uint8_t* buffer, uint8_t len);
static uint8_t generate_header(Modbus485Device_t* device, uint8_t id, uint16_t reg, uint16_t len, uint8_t* out);

// Public API

uint8_t pce_modbus_ascii_generate_read_packet(Modbus485Device_t* device, uint16_t reg, uint16_t len, uint8_t* out)
{
    // packet header
    uint8_t i = generate_header(device, PCE_FRAME_READ, reg, len, out);
    
    // crc
    uint8_t crc = calculate_crc(out, i);
    out[i++] = crc;
    out[i++] = PCE_PROTOCOL_END;
    
    return i;   // return size of packet
}

// Frame examples
// Master (address 0) register 0 (display) to slave 28
//  STX ID  RSV FROM    TO  REG RSV LONG    CRC EXT
//  2   36  32  32      60  32  32  32      58  3

// response
//  STX ID  RSV FROM    TO  REG RSV LONG    DATA                                CRC ETX
//  2   37  32  60      32  32  32  40      43  48  55  54  53  46  52  51      15  3

// error frame
// Slave 11 to master 0 with error frame indicating unknown register (in REG field)
//  STX ID  RSV FROM    TO  REG RSV LONG    CRC EXT
//  2   38  32  43      32  33  32  32      46  3

/*
int8 Calculate_CRC(int8 CRC_Position)
{
 int8 i,CRC=0;
 for(i=0;c<CRC_Position;c++)
 {
 crc=crc ^ frame[i];
 }
 if(crc<32) CRC=~CRC;
 return(CRC);
}
*/

//uint8_t frame[8] = {2, 36, 32, 32, 60, 32, 32, 32};
//uint8_t frame[16] = {2, 37, 32, 60, 32, 32, 32, 40,  43, 48, 55, 54, 53, 46, 52, 51};

// Static functions

static uint8_t calculate_crc(unsigned char* frame, int8_t len)
{
    uint8_t crc = 0;

    for(uint8_t i=0; i < len;i++)
    {
    crc = crc ^ frame[i];
    }

    if(crc < 32) crc=~crc;
    return(crc);
}

static uint8_t check_crc(uint8_t* buffer, uint8_t len)
{
    uint8_t crc = calculate_crc(buffer, len-2);
    
    if((crc) == buffer[len-2])
        return 1;
    return 0;
}

static uint8_t generate_header(Modbus485Device_t* device, uint8_t id, uint16_t reg, uint16_t len, uint8_t* out)
{
    uint8_t i = 0;
    len = 0;    // set artifially to 0 because no needed
    out[i++] = PCE_PROTOCOL_START;                  // Stx
    out[i++] = id;                                  // Id
    out[i++] = PCE_PROTOCOL_RSV;                    // Rsv
    out[i++] = PCE_PROTOCOL_CONV + 0;               // From master (0: real value + 32)
    out[i++] = PCE_PROTOCOL_CONV + device->address; // To slave (address: real value + 32)
    out[i++] = PCE_PROTOCOL_CONV + reg;             // Register (word: real value + 32)
    out[i++] = PCE_PROTOCOL_RSV;                    // Rsv
    out[i++] = PCE_PROTOCOL_CONV + len;             // Long len (real value + 32)
    return i;   // return size of packet header
}

// send frame function
// parse frame function