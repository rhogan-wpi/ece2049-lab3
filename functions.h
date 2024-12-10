#include <stdlib.h>
#include <msp430.h>
#include <stdint.h>
#include "peripherals.h"
#define MOVING_AVERAGE_SIZE 36

// Temperature Sensor Calibration = Reading at 30 degrees C is stored at addr 1A1Ah
// See end of datasheet for TLV table memory mapping
#define CALADC12_15V_30C *((unsigned int *)0x1A1A)
// Temperature Sensor Calibration = Reading at 85 degrees C is stored at addr 1A1Ch
//See device datasheet for TLV table memory mapping
#define CALADC12_15V_85C *((unsigned int *)0x1A1C)
#define ONE_MONTH_IN_ADC 342 // 12 segments from 0 --> 2045

// Enum for the game state
typedef enum {
  DISPLAY,
  EDIT,
} state;

// Rin Timer A2
void runtimerA2(void);

// Display functions
// Display the Date
void displayDate(char date[7], volatile long unsigned int, volatile unsigned int, volatile unsigned int);

// Display the Time
void displayTime(char time[9], volatile long unsigned int, volatile unsigned int, volatile unsigned int, volatile unsigned int);

// Display the Temp in C
void displayTempC(char tempC[7], volatile float);

// Display the Temp in F
void displayTempF(char tempF[7], volatile float);

// ADC functions
// Configure the ADC12
void config_ADC(volatile float, volatile unsigned int, volatile unsigned int);

// Convert ADC 2 Time --> Populate slider
volatile unsigned int ADC_2_Time(void);

// Convert ADC 2 Temp --> Populate in_temp
volatile unsigned int ADC_2_Temp(void);

// Buttons functions
// Init user's launchpad buttons
void init_launchpad_button();

// Read user's launchpad button
unsigned int read_launchpad_button();






// Initializes the two user LEDs
void init_user_leds();

//Sets the two user LEDs
void set_user_leds(unsigned char uled);

// Initializes the buttons for input
void init_board_buttons();

//Reads the four buttons
unsigned int read_board_buttons();
