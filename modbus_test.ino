#define MAX485_DE_RE 7
#define BAUD_RATE    19200

#include <Arduino.h>
#include "plib_modbus485.h"
#include "micra_e6_modbus485_protocol.h"
#include "pce_modbus_ascii_protocol.h"

uint8_t tx_buffer[64];
uint8_t rx_buffer[64];
uint8_t rx_len = 0;
uint8_t g_current_slave_id = 0;

// only for debugging
uint8_t g_micra_conf[] = {0, 2};
uint8_t g_delay_conf[] = {2, 2};
uint32_t lastTime = 0;

// gpio setters
void setTX() { digitalWrite(MAX485_DE_RE, HIGH); }  // driver ON
void setRX() { digitalWrite(MAX485_DE_RE, LOW); }   // driver OFF, receiver ON

Modbus485Device_t deviceList[] =
{
  {
      .address = 1,
      .last_word = 0,
  },
  {
      .address = 12,
      .last_word = 0,
  },
  {
      .address = 3,
      .last_word = 0,
  },
  {
      .address = 28,
      .last_word = 0,
  }
};

uint8_t read_input_packet[] = {0x01, 0x03, 0x02, 0x01, 0x00, 0xb9, 0xd4};
uint8_t read_display_pakt[] = {0x01, 0x03, 0x02, 0x09, 0x60, 0xbe, 0x3c};

uint8_t read_pce_display_packet[] = {2, 37, 32, 60, 32, 32, 32, 40,   43, 48, 55, 54, 53, 46, 52, 51,   53, 3};
uint8_t read_pce_display_packet2[] = {2, 37, 32, 60, 32, 32, 32, 40,   43, 49, 46, 49, 50, 53, 48, 48,   49, 3};

void setup()
{
  // Init hardware
  pinMode(MAX485_DE_RE, OUTPUT);
  setRX();  // start in RX

  // Init serial
  Serial.begin(115200);
  Serial1.begin(BAUD_RATE, SERIAL_8N1);
  Serial2.begin(BAUD_RATE, SERIAL_8N1);

  // Init devices
  deviceList[0].parser.Write_Words = micra_modbus485_generate_write_packet;
  deviceList[0].parser.Read_Words = micra_modbus485_generate_read_packet;
  deviceList[0].parser.Parse_Response = micra_modbus485_parse_response;

  deviceList[1].parser.Write_Words = micra_modbus485_generate_write_packet;
  deviceList[1].parser.Read_Words = micra_modbus485_generate_read_packet;
  deviceList[1].parser.Parse_Response = micra_modbus485_parse_response;

  deviceList[2].parser.Write_Words = micra_modbus485_generate_write_packet;
  deviceList[2].parser.Read_Words = micra_modbus485_generate_read_packet;
  deviceList[2].parser.Parse_Response = micra_modbus485_parse_response;

  deviceList[3].parser.Write_Words = nullptr;
  deviceList[3].parser.Read_Words = pce_modbus_ascii_generate_read_packet;
  deviceList[3].parser.Parse_Response = pce_modbus_ascii_parse_response;
}

void loop()
{
  Modbus485Device_t* current_device = &deviceList[g_current_slave_id];

  // just for testing
  while(Serial.available())
  {
    int8_t option = Serial.read();

    switch (option)
    {
      // change current slave address
      case '0':
        g_current_slave_id = 0;
        break;
      
      case '1':
        g_current_slave_id = 1;
        break;
      
      case '2':
        g_current_slave_id = 2;
        break;
      
      case '3':
        g_current_slave_id = 3;
        break;
      
      // send packet (micra)
      case 'a':
        //current_device->last_word = MICRA_MODBUS_REG_INPUT_RANGE; // todo: delete after test
        //Serial2.write(read_input_packet, 7);                      // todo: delete after test
        sendModbusReadCommand(current_device, MICRA_MODBUS_REG_INPUT_RANGE);
        break;
      
      case 'b':
        //current_device->last_word = MICRA_MODBUS_REG_DISPLAY;   // todo: delete after test
        //Serial2.write(read_display_pakt, 7);                    // todo: delete after test
        sendModbusReadCommand(current_device, MICRA_MODBUS_REG_DISPLAY);
        break;
      
      case 'c':
        sendModbusWriteCommand(current_device, MICRA_MODBUS_REG_BRIGHTNESS_ROUND, g_micra_conf);
        break;
      
      case 'd':
        sendModbusWriteCommand(current_device, MICRA_MODBUS_REG_RS_PROTOCOL_DELAY, g_delay_conf);
        break;
      
      case 'e':     
        sendModbusReadCommand(current_device, MICRA_MODBUS_REG_RS_PROTOCOL_DELAY);
        break;
      
      // send packet (frequency)
      case 'z':
        //current_device->last_word = PCE_REGISTER_DISPLAY1;
        //Serial2.write(read_pce_display_packet2, 18);
        sendModbusReadCommand(current_device, PCE_REGISTER_DISPLAY1);
        break;
      
      default:
        break;
    }
  }

  // parser
  while(Serial1.available())
  {
    rx_buffer[rx_len++] = Serial1.read();

    if(!current_device->parser.Parse_Response)
      return;
    
    uint8_t res = current_device->parser.Parse_Response(current_device, rx_buffer, rx_len);

    if(res == MODBUS_ERR_BAD_LEN)
      return;
    
    printBuffer(rx_buffer, rx_len, "RX");

    switch (res)
    {      
      case MODBUS_ERR_BAD_SLAVE_ADDRESS:
        Serial.println("Bad slave address");
        break;
      
      case MODBUS_ERR_RESPONSE:
        Serial.println("Error received: " + String(current_device->error));
        break;
      
      case MODBUS_ERR_BAD_CRC_ERROR:
        Serial.println("Bad CRC Error");
        break;
      
      case MODBUS_ERR_BAD_CRC_WRITE:
        Serial.println("Bad CRC Write");
        break;
      
      case MODBUS_ERR_BAD_CRC_READ:
        Serial.println("Bad CRC Read");
        break;
      
      case MODBUS_ERR_OTHER:
        Serial.println("Other error");
        break;
      
      case MODBUS_OK:
        Serial.println("Measure: " + String(current_device->measure));
        break;
    }
    
    clearSerialPort();
  }
}

void printBuffer(uint8_t* buffer, uint8_t len, char* name)
{
  Serial.print("[len: " + String(len) + "]\t[" + String(name) + ": ");

  for(uint8_t i = 0; i < len; i++)
  {
    Serial.print(buffer[i] + String(" "));
  }
  Serial.println("]");
}

void sendModbusCommand(uint8_t* data, uint8_t len)
{
  setTX();                          // tx mode
  Serial1.write(tx_buffer, len);    // send data
  Serial1.flush();                  // wait tranmission is done
  setRX();                          // rx mode
  clearSerialPort();                // clear rx buffer
  printBuffer(tx_buffer, len, "TX");
}

void sendModbusReadCommand(Modbus485Device_t* device, uint8_t word)
{
  // Prepare packet to send
  if(device->parser.Read_Words)
  {
    uint8_t bytes_to_write = device->parser.Read_Words(device, word, 1, tx_buffer);
    device->last_word = word;
    // Transmit packet
    sendModbusCommand(tx_buffer, bytes_to_write);
  }
}

void sendModbusWriteCommand(Modbus485Device_t* device, uint8_t word, uint8_t* data)
{
  // Prepare packet to send
  if(device->parser.Write_Words)
  {
    uint8_t bytes_to_write = device->parser.Write_Words(device, word, 1, data, tx_buffer);
    device->last_word = word;
    // Transmit packet
    sendModbusCommand(tx_buffer, bytes_to_write);
  }
}

void clearSerialPort()
{
  while (Serial1.available() > 0)
  {
    Serial1.read();
  }
  rx_len = 0;
}
