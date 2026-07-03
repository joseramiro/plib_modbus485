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
static uint8_t generate_header(Modbus485Comm_t* comm, uint8_t id, uint16_t reg, uint16_t len, uint8_t* out);
static void update_struct(Modbus485Comm_t* comm, uint8_t* raw_data);
static uint16_t convert_ascii_to_int(uint8_t* raw_data);

// Public API

uint8_t pce_modbus_ascii_generate_read_packet(Modbus485Comm_t* comm, uint16_t reg, uint16_t len, uint8_t* out)
{
    // packet header
    uint8_t i = generate_header(comm, PCE_FRAME_READ, reg, len, out);
    
    // crc
    uint8_t crc = calculate_crc(out, i);
    out[i++] = crc;
    out[i++] = PCE_PROTOCOL_END;
    
    return i;   // return size of packet
}

uint8_t pce_modbus_ascii_parse_response(Modbus485Comm_t* comm, uint8_t* rx, uint8_t rx_len)
{
    // check min size to be 10: true continue, false wait until size is ok
    if(rx_len < 10)
        return MODBUS_ERR_BAD_LEN;

    // check address
    //if(rx[PCE_POSITION_HEADER_FROM] != comm->address + PCE_PROTOCOL_CONV)
    //    return MODBUS_ERR_BAD_SLAVE_ADDRESS;

    // check error frame
    if(rx[PCE_POSITION_HEADER_ID] == PCE_FRAME_ERR)
    {
        // check crc
        if(check_crc(rx, 8))
        {
            //comm->error = rx[PCE_POSITION_HEADER_REG];
            return MODBUS_ERR_RESPONSE;
        }
        else
            return MODBUS_ERR_BAD_CRC_ERROR;
    }
    
    // check read frame
    if(rx[PCE_POSITION_HEADER_ID] == PCE_FRAME_ANS)
    {
        if(rx_len >= 18)
        {
            // check crc
            if(check_crc(rx, 18))
            {
                comm->last_word = rx[PCE_POSITION_HEADER_REG] - PCE_PROTOCOL_CONV;
                update_struct(comm, &rx[8]);
                comm->response_received = 1;
                return MODBUS_OK;
            }
            else
                return MODBUS_ERR_BAD_CRC_READ;
        }
        else
        {
            return MODBUS_ERR_BAD_LEN;
        }
    }
    
    return MODBUS_ERR_OTHER;
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

static uint8_t generate_header(Modbus485Comm_t* comm, uint8_t id, uint16_t reg, uint16_t len, uint8_t* out)
{
    uint8_t i = 0;
    len = 0;    // set artifially to 0 because no needed
    out[i++] = PCE_PROTOCOL_START;                  // Stx
    out[i++] = id;                                  // Id
    out[i++] = PCE_PROTOCOL_RSV;                    // Rsv
    out[i++] = PCE_PROTOCOL_CONV + 0;               // From master (0: real value + 32)
    out[i++] = PCE_PROTOCOL_CONV + comm->address; // To slave (address: real value + 32)
    out[i++] = PCE_PROTOCOL_CONV + reg;             // Register (word: real value + 32)
    out[i++] = PCE_PROTOCOL_RSV;                    // Rsv
    out[i++] = PCE_PROTOCOL_CONV + len;             // Long len (real value + 32)
    return i;   // return size of packet header
}

static void update_struct(Modbus485Comm_t* comm, uint8_t* raw_data)
{
    switch(comm->last_word)
    {            
        case PCE_REGISTER_DISPLAY1:
            comm->measure = convert_ascii_to_int(raw_data);
            break;
    };
}

// convert ascii to int (exemple +1.12500 will be 11250)
static uint16_t convert_ascii_to_int(uint8_t* raw_data)
{
    uint32_t value = 0;
    uint8_t after_dot = 0, decimals = 0;
    uint8_t *s = raw_data;

    // Skip optional sign
    if (*s == '+' || *s == '-')
        s++;

    while (*s)
    {
        if (*s >= '0' && *s <= '9')
        {
            value = value * 10 + (*s - '0');
            if(after_dot)
                decimals++;
        }
        // ignore decimal point
        else if (*s == '.')
        {
            if(!after_dot)
                after_dot = 1;
            else
                break;
        }
        else
        {
            break;
        }
        s++;
    }

    // We want exactly 4 decimal places (scale factor 10000)
    // Get the 4 decimals
    while (decimals < 4)
    {
        value *= 10;
        decimals++;
    }
    // Truncate the extra decimals
    while (decimals > 4)
    {
        value /= 10;
        decimals--;
    }

    return value;
}