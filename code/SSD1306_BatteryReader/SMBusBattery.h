#ifndef SMBUS_BATTERY
#define SMBUS_BATTERY

#include <Arduino.h>

// SMBus address for BMS
#define BATTERY_ADDR 0x0B

// the size of the data array
#define BATT_BUFLEN 32

int sbsWord       (byte function);
byte sbsBlock     (byte function, byte* blockBuffer);
void sbsGetData   (void);
void sbsPrintData (void);

// Smart Battery functions and command registers (see BMS datasheet Table 3)
#define BATTERY_MODE             0x03
#define RATE                     0x04
#define TEMPERATURE              0x08
#define VOLTAGE                  0x09
#define CURRENT                  0x0A
#define RELATIVE_SOC             0x0D
#define ABSOLUTE_SOC             0x0E
#define REMAINING_CAPACITY       0x0F
#define FULL_CHARGE_CAPACITY     0x10
#define TIME_TO_EMPTY            0x12
#define TIME_TO_FULL             0x13
#define CHARGING_CURRENT         0x14
#define CHARGING_VOLTAGE         0x15
#define BATTERY_STATUS           0x16
#define CYCLE_COUNT              0x17
#define DESIGN_CAPACITY          0x18
#define DESIGN_VOLTAGE           0x19
#define SPEC_INFO                0x1A
#define MFG_DATE                 0x1B
#define SERIAL_NUM               0x1C
#define MFG_NAME                 0x20   
#define DEV_NAME                 0x21   
#define CELL_CHEM                0x22   
#define CELL4_VOLTAGE            0x3C 
#define CELL3_VOLTAGE            0x3D
#define CELL2_VOLTAGE            0x3E
#define CELL1_VOLTAGE            0x3F

// Found that current and amp-hour data from BMS needed to be multiplied by this factor to be accurate
#define CURRENT_FACTOR           2.15

extern float temperature;
extern uint16_t design_capacity,
temperature_k,
voltage,
current,
relative_soc,
absolute_soc,
remaining_capacity,
time_to_empty,
time_to_full,
cycle_count,
cell1_voltage,
cell2_voltage,
cell3_voltage,
cell4_voltage;

#endif