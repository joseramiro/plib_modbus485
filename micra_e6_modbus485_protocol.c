#include "micra_e6_modbus485_protocol.h"

// Static functions
static uint16_t calculate_crc16(uint8_t* buffer, uint8_t len);
static uint8_t check_crc16(uint8_t* buffer, uint8_t len);
static uint8_t generate_header(Modbus485Device_t* device, uint8_t mode, uint16_t first_word, uint16_t nb_words, uint8_t* out);
static void update_struct(Modbus485Device_t* device, uint8_t* raw_data);

// Public API
uint8_t micra_modbus485_generate_read_packet(Modbus485Device_t* device, uint16_t first_word, uint16_t nb_words, uint8_t* out)
{
    // packet header
    uint8_t i = generate_header(device, MODBUS485_MODE_READ_WORD, first_word, nb_words, out);
    
    // crc
    uint16_t crc = calculate_crc16(out, i);
    out[i++] = crc;
    out[i++] = (crc >> 8);
    
    return i;   // return size of packet
}

uint8_t micra_modbus485_generate_write_packet(Modbus485Device_t* device, uint16_t first_word, uint16_t nb_words, uint8_t* data, uint8_t* out)
{
    // packet header
    uint8_t i = generate_header(device, MODBUS485_MODE_WRITE_WORD, first_word, nb_words, out);
    
    // word size in bytes
    uint8_t words_size = nb_words * 2;
    out[i++] = words_size;
    memcpy(&out[i], data, words_size);
    i += words_size;
    
    // crc
    uint16_t crc = calculate_crc16(out, i);
    out[i++] = (uint8_t)crc;
    out[i++] = (crc >> 8);
    
    return i;   // return packet size
}

uint8_t micra_modbus485_parse_response(Modbus485Device_t* device, uint8_t* rx, uint8_t rx_len)
{
    // check min size to be 5: true continue, false wait until size is ok
    if(rx_len < 5)
        return MODBUS_ERR_BAD_LEN;
        
    // check address
    if(rx[MICRA_E6_MODBUS485_POSITION_ADDRESS] != device->address)
        return MODBUS_ERR_BAD_SLAVE_ADDRESS;
        
    // check mode is error (read word + 0x80 or write word + 0x08):
    if((rx[MICRA_E6_MODBUS485_POSITION_MODE] == (MODBUS485_MODE_READ_WORD | MODBUS485_MODE_ERROR)) ||
       (rx[MICRA_E6_MODBUS485_POSITION_MODE] == (MODBUS485_MODE_WRITE_WORD | MODBUS485_MODE_ERROR)))
    {
        // check crc
        if(check_crc16(rx, 5))
        {
            device->error = rx[2];
            return MODBUS_ERR_RESPONSE;
        }
        else
            return MODBUS_ERR_BAD_CRC_ERROR;
    }

    // check mode is write word
    else if(rx[MICRA_E6_MODBUS485_POSITION_MODE] == MODBUS485_MODE_WRITE_WORD)
    {
        if(rx_len < 8)
            return MODBUS_ERR_BAD_LEN;
        // check crc
        if(check_crc16(rx, 8))
        {
            return MODBUS_OK;
        }
        else
            return MODBUS_ERR_BAD_CRC_WRITE;
        
    }
    // check mode is read Words
    else if(rx[MICRA_E6_MODBUS485_POSITION_MODE] == MODBUS485_MODE_READ_WORD)
    {
        if(rx_len < (5 + rx[2]))
            return MODBUS_ERR_BAD_LEN;
        // check crc
        if(check_crc16(rx, 5 + rx[2]))
        {
            update_struct(device, &rx[3]);
            return MODBUS_OK;
        }
        else
            return MODBUS_ERR_BAD_CRC_READ;
    }
    
    return MODBUS_ERR_OTHER;
}


// Static functions
static uint16_t calculate_crc16(uint8_t* buffer, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    
    for(uint8_t i = 0; i < len; i++)
    {
        crc = crc^(uint8_t)buffer[i];
        
        for(uint8_t j = 0 ; j < 8 ; j++)
    	{
            if(crc & 0x0001)
    		{
    			crc = (crc>>1);
    			crc &= 0x7FFF;
    			crc = crc^0xA001;
    		}
            else
    		{
            	crc >>= 1;
    			crc &= 0x7FFF;
    		}
    	}
    }
    
    return crc;
}

static uint8_t check_crc16(uint8_t* buffer, uint8_t len)
{
    uint16_t crc = calculate_crc16(buffer, len-2);
    
    if((0x00ff & crc) == buffer[len-2] && ((crc) >> 8) == buffer[len-1])
        return 1;
    return 0;
}

static uint8_t generate_header(Modbus485Device_t* device, uint8_t mode, uint16_t first_word, uint16_t nb_words, uint8_t* out)
{
    uint8_t i = 0;
    out[i++] = device->address;
    out[i++] = mode;
    out[i++] = first_word >> 8;
    out[i++] = first_word;
    out[i++] = nb_words >> 8;
    out[i++] = nb_words;
    return i;   // return size of packet header
}

static void update_struct(Modbus485Device_t* device, uint8_t* raw_data)
{
    switch(device->last_word)
    {
        case MICRA_MODBUS_REG_INPUT_RANGE:
            device->input = raw_data[0];
            device->range = raw_data[1];
            break;
            
        case MICRA_MODBUS_REG_DISPLAY:
            device->measure = raw_data[0] << 8 | raw_data[1];
            break;
    };
}