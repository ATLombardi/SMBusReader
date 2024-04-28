/*  
SMBus Battery Management System (BMS) Reader
Tested with Ironworks 1240ST which uses the TI BQ2085 SBS 1.1 compliant gas gauge
  (https://www.ti.com/product/BQ2085)
By Paul Martis - W9PEM
w9pem@digitalmisery.com
4 February 2024
*/

#include "SMBusBattery.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>

uint8_t smbusBuffer[BATT_BUFLEN];

uint16_t design_capacity = 0;
uint16_t temperature_k = 0;
float temperature = 0;
uint16_t voltage = 0;
uint16_t current = 0;
uint16_t relative_soc = 0;
uint16_t absolute_soc = 0;
uint16_t remaining_capacity = 0;
uint16_t time_to_empty = 0;
uint16_t time_to_full = 0;
uint16_t cycle_count = 0;
uint16_t cell1_voltage = 0;
uint16_t cell2_voltage = 0;
uint16_t cell3_voltage = 0;
uint16_t cell4_voltage = 0;

int sbsWord (byte function) {
  Wire.beginTransmission(BATTERY_ADDR);
  Wire.write(function);
  Wire.endTransmission(function); // for better compat with Wire Library, send stop
  delayMicroseconds(100);         //   and a delay before requesting data
  Wire.requestFrom(BATTERY_ADDR, 2);
  byte low  = Wire.read();
  byte high = Wire.read();
  return ((int)high) << 8 | (int)low;
}

byte sbsBlock (byte function, byte* blockBuffer) {
  byte x, num_bytes;
  Wire.beginTransmission(BATTERY_ADDR);
  Wire.write(function);
  Wire.endTransmission();
  delayMicroseconds(100);
  Wire.requestFrom(BATTERY_ADDR, 1); // first byte is the block size
  num_bytes = Wire.read();
  Wire.beginTransmission(BATTERY_ADDR); // start again now that we know the size
  Wire.write(function);
  Wire.endTransmission();
  delayMicroseconds(100);
  Wire.requestFrom(BATTERY_ADDR, num_bytes+1);
  Wire.read(); // throw size byte away, we know it already
  for (x=0; x < num_bytes; x++) {
    smbusBuffer[x] = Wire.read();
  }
  smbusBuffer[x] = 0; // add null at end
  return num_bytes;
}

void sbsGetData (void) {
  // Get dynamic values
  temperature_k = sbsWord(TEMPERATURE);
  temperature = (float)temperature_k/10.0-273.15;
  voltage = sbsWord(VOLTAGE);
  current = sbsWord(CURRENT);
  relative_soc = sbsWord(RELATIVE_SOC);
  absolute_soc = sbsWord(ABSOLUTE_SOC);
  remaining_capacity = sbsWord(REMAINING_CAPACITY);
  time_to_empty = sbsWord(TIME_TO_EMPTY);
  time_to_full = sbsWord(TIME_TO_FULL);
  cycle_count = sbsWord(CYCLE_COUNT);
  cell1_voltage = sbsWord(CELL1_VOLTAGE);
  cell2_voltage = sbsWord(CELL2_VOLTAGE);
  cell3_voltage = sbsWord(CELL3_VOLTAGE);
  cell4_voltage = sbsWord(CELL4_VOLTAGE);
}

// Print BMS data to Serial Port
void sbsPrintData (void) {
  uint8_t block_length = 0;
  uint16_t abs_current = 0;
  // Print Static Battery Data
  Serial.print("MFG Name: ");
  block_length = sbsBlock(MFG_NAME, smbusBuffer);
  Serial.write(smbusBuffer, block_length);
  Serial.println(" ");
  Serial.print("Device Name: ");
  block_length = sbsBlock(DEV_NAME, smbusBuffer);
  Serial.write(smbusBuffer, block_length);
  Serial.println(" ");
  Serial.print("Chemistry: ");
  block_length = sbsBlock(CELL_CHEM, smbusBuffer);
  Serial.write(smbusBuffer, block_length);
  Serial.println(" ");
  Serial.print("Design Capacity: ");
  Serial.print(design_capacity);
  Serial.println("mAh");
  Serial.print("Current Capacity: ");
  Serial.print((int)(sbsWord(FULL_CHARGE_CAPACITY)*CURRENT_FACTOR));
  Serial.println("mAh");
  Serial.print("Design Voltage: ");
  Serial.print(sbsWord(DESIGN_VOLTAGE));
  Serial.println("mV");
  String formatted_date = "Manufacture Date (Y-M-D): ";
  int mdate = sbsWord(MFG_DATE);
  int mday = 0b00011111 & mdate;
  int mmonth = mdate>>5 & 0b00001111;
  int myear = 1980 + (mdate>>9 & 0b01111111);
  formatted_date += myear;
  formatted_date += "-";
  formatted_date += mmonth;
  formatted_date += "-";
  formatted_date += mday;
  Serial.println(formatted_date);
  Serial.print("Serial Number: ");
  Serial.println(sbsWord(SERIAL_NUM));
  Serial.print("Battery Mode (BIN): 0b");
  Serial.println(sbsWord(BATTERY_MODE),BIN);
  Serial.print("Battery Status (BIN): 0b");
  Serial.println(sbsWord(BATTERY_STATUS),BIN);
  Serial.println("");
  Serial.println("*************Battery Data*************");
  Serial.print("Voltage: ");
  Serial.print((float)voltage/1000);
  Serial.println("V");
  if (current > 50000)
  {
    abs_current = 0 - current;
    Serial.print("Discharge: ");
    Serial.print((int)(abs_current*CURRENT_FACTOR));
    Serial.println("mA");
    Serial.print("Minutes remaining until empty: ");
    Serial.println(time_to_empty);
  }
  else
  {
    abs_current = current;
    Serial.print("Charge: ");
    Serial.print((int)(abs_current*CURRENT_FACTOR));
    Serial.println("mA");
    if (abs_current != 0)
    {
      Serial.print("Minutes remaining for full charge: ");
      Serial.println(time_to_full);
    }
  }

  Serial.print("Remaining Capacity: " );
  Serial.print((int)(remaining_capacity*CURRENT_FACTOR));
  Serial.println("mAh");

  Serial.print("Relative SOC: ");
  Serial.print(relative_soc);
  Serial.print("%");
  Serial.print("   ");
  
  Serial.print("Absolute SOC: ");
  Serial.print(absolute_soc);
  Serial.println("%");
  
  Serial.print("Cycle Count: " );
  Serial.println(cycle_count);

  Serial.print("Temp: ");
  Serial.print((float)temperature);
  Serial.println("C");
  
  Serial.println("Cell Voltages:");
  Serial.print("1: ");
  Serial.print((float)cell1_voltage/1000);
  Serial.print("V 2: ");
  Serial.print((float)cell2_voltage/1000);
  Serial.print("V 3: ");
  Serial.print((float)cell3_voltage/1000);
  Serial.print("V 4: ");
  Serial.print((float)cell4_voltage/1000);
  Serial.println("V");
  
  Serial.println("**************************************\n");
}