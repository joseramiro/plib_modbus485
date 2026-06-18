#define MAX485_DE_RE 7
#define BAUD_RATE    19200

#include <Arduino.h>
#include "plib_modbus485.h"
#include "micra_e6_modbus485_protocol.h"

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
      .address = 4,
      .last_word = 0,
  }
};

void setup()
{
  // Init hardware
  pinMode(MAX485_DE_RE, OUTPUT);
  setRX();  // start in RX

  // Init serial
  Serial.begin(115200);
  Serial1.begin(BAUD_RATE, SERIAL_8E1);

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

  //deviceList[3].parser.Write_Words = micra_modbus485_generate_write_packet;
  //deviceList[3].parser.Read_Words = micra_modbus485_generate_read_packet;
  //deviceList[3].parser.Parse_Response = micra_modbus485_parse_response;
}

void loop()
{
  Modbus485Device_t* current_device = &deviceList[g_current_slave_id];
  // timer only for testing
  /*
  uint32_t now = millis();
  if(now - lastTime >= 1000)
  {
    lastTime = now;
    uint8_t read_display[] = {0x01, 0x03, 0x02, 0x09, 0x60, 0xbe, 0x3c};
    uint8_t rx_len = sizeof(read_display);
    uint8_t res = current_device->parser.Parse_Response(current_device, read_display, rx_len);
    Serial.println(res);
    if(res == MODBUS_OK)
    {
      Serial.print("Measure: ");
      Serial.print(current_device->measure);
      Serial.println();
    }
  }
  */

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
        g_current_slave_id = 4;
        break;
      
      // send packet (micra)
      case 'a':
        sendModbusReadCommand(current_device, MICRA_MODBUS_REG_INPUT_RANGE);
        break;
      
      case 'b':
        sendModbusReadCommand(current_device, MICRA_MODBUS_REG_DISPLAY);
        break;
      
      case 'c':
        sendModbusWriteCommand(current_device, MICRA_MODBUS_REG_BRIGHTNESS_ROUND, g_micra_conf);
        break;
      
      // send packet (frequency)
      case 'z':
        break;
      
      default:
        break;
    }
  }

  while(Serial1.available())
  {
    Serial.print("len: ");
    Serial.println(rx_len);
    rx_buffer[rx_len++] = Serial1.read();
    uint8_t res = current_device->parser.Parse_Response(current_device, rx_buffer, rx_len);
    switch (res)
    {
      case MODBUS_ERR_BAD_LEN:
        Serial.println("Waiting packet...");
        break;
      
      case MODBUS_ERR_BAD_SLAVE_ADDRESS:
        Serial.println("Bad slave address");
        Serial1.flush();
        rx_len = 0;
        break;
      
      case MODBUS_ERR_RESPONSE:
        Serial.print("Error received: ");
        Serial.print(current_device->error);
        Serial.println();
        Serial1.flush();
        rx_len = 0;
        break;
      
      case MODBUS_ERR_BAD_CRC:
        Serial.println("Bad CRC");
        Serial1.flush();
        rx_len = 0;
        break;
      
      case MODBUS_ERR_OTHER:
        Serial.println("Other error");
        Serial1.flush();
        rx_len = 0;
        break;
      
      case MODBUS_OK:
        Serial.print("Measure: ");
        Serial.print(current_device->measure);
        Serial.println();
        Serial1.flush();
        rx_len = 0;
        break;
      
    }
  }

}

void sendModbusCommand(uint8_t* data, uint8_t len)
{
  Serial1.flush();
  setTX();
  Serial1.write(tx_buffer, len);
  setRX();
}

void sendModbusReadCommand(Modbus485Device_t* device, uint8_t word)
{
  // Prepare packet to send
  uint8_t bytes_to_write = device->parser.Read_Words(device, word, 1, tx_buffer);
  device->last_word = word;
  // Transmit packet
  sendModbusCommand(tx_buffer, bytes_to_write);
}

void sendModbusWriteCommand(Modbus485Device_t* device, uint8_t word, uint8_t* data)
{
  // Prepare packet to send
  uint8_t bytes_to_write = device->parser.Write_Words(device, word, 1, data, tx_buffer);
  device->last_word = word;
  // Transmit packet
  sendModbusCommand(tx_buffer, bytes_to_write);
}
