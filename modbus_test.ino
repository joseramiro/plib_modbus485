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
      .address = 2,
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

void setup()
{
  // Init hardware
  pinMode(MAX485_DE_RE, OUTPUT);
  setRX();  // start in RX

  // Init serial
  Serial.begin(115200);
  Serial1.begin(BAUD_RATE, SERIAL_8E1);
  Serial2.begin(BAUD_RATE, SERIAL_8E1);

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
  deviceList[3].parser.Parse_Response = nullptr;
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
        current_device->last_word = MICRA_MODBUS_REG_INPUT_RANGE;
        Serial2.write(read_input_packet, 7);
        //sendModbusReadCommand(current_device, MICRA_MODBUS_REG_INPUT_RANGE);
        break;
      
      case 'b':
        current_device->last_word = MICRA_MODBUS_REG_DISPLAY;
        Serial2.write(read_display_pakt, 7);
        //sendModbusReadCommand(current_device, MICRA_MODBUS_REG_DISPLAY);
        break;
      
      case 'c':
        sendModbusWriteCommand(current_device, MICRA_MODBUS_REG_BRIGHTNESS_ROUND, g_micra_conf);
        break;
      
      // send packet (frequency)
      case 'z':
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

    printBuffer(rx_buffer, 20, "RX");
    
    if(!current_device->parser.Parse_Response)
      return;
    
    uint8_t res = current_device->parser.Parse_Response(current_device, rx_buffer, rx_len);

    if(res == MODBUS_ERR_BAD_LEN)
      return;

    switch (res)
    {      
      case MODBUS_ERR_BAD_SLAVE_ADDRESS:
        Serial.println("Bad slave address");
        clearSerialPort();
        break;
      
      case MODBUS_ERR_RESPONSE:
        Serial.print("Error received: ");
        Serial.print(current_device->error);
        Serial.println();
        clearSerialPort();
        break;
      
      case MODBUS_ERR_BAD_CRC_ERROR:
        Serial.println("Bad CRC Error");
        clearSerialPort();
        break;
      
      case MODBUS_ERR_BAD_CRC_WRITE:
        Serial.println("Bad CRC Write");
        clearSerialPort();
        break;
      
      case MODBUS_ERR_BAD_CRC_READ:
        Serial.println("Bad CRC Read");
        clearSerialPort();
        break;
      
      case MODBUS_ERR_OTHER:
        Serial.println("Other error");
        clearSerialPort();
        break;
      
      case MODBUS_OK:
        Serial.print("Measure: ");
        Serial.print(current_device->measure);
        Serial.println();
        clearSerialPort();
        break;
    }
  }

}

void printBuffer(uint8_t* buffer, uint8_t len, char* name)
{
  Serial.print("[len: ");
  Serial.print(len);
  Serial.print("]\t[");
  Serial.print(name);
  Serial.print(": ");
  for(uint8_t i = 0; i < len; i++)
  {
    Serial.print(buffer[i]);
    Serial.print(" ");
  }
  Serial.println("]");
}

void sendModbusCommand(uint8_t* data, uint8_t len)
{
  Serial2.flush();
  setTX();
  Serial2.write(tx_buffer, len);
  setRX();
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
  Serial1.flush();
  rx_len = 0;
}
