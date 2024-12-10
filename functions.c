#include "functions.h"

// User written functions

//Global timer A2 for clock, frequency is 32768 with an /8 division interrupt timing is 1 second.
void runtimerA2(void) {
  TA2CTL = TASSEL_1 | MC_1 | ID_3;
  TA2CCR0 = 4095;
  TA2CCTL0 = CCIE;
}

// Display date
void displayDate(char date[7], volatile long unsigned int global_counter, volatile unsigned int adc_month, volatile unsigned int adc_date) {
  const char* month_abbr[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  //Stores length of month to be used to decrement days later
  const int month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  // unsigned int month = adc_month;
  unsigned int date = adc_date + (global_counter / 86400);

  if (date > month_days[adc_month - 1]) {
    date -= month_days[adc_month - 1];
    adc_month ++;
    adc_date = 0;
    if (adc_month > 12) {
      adc_month = 1; // Restart for a new year --> doesn't need adc_month anymore

    }
  }

  char day_tens = ((date - (date % 10)) / 10) + '0';
  char day_ones = (date % 10) + '0';
  date[0] = month_abbr[adc_month - 1][0];
  date[1] = month_abbr[adc_month - 1][1];
  date[2] = month_abbr[adc_month - 1][2];
  date[3] = ' ';
  date[4] = day_tens;
  date[5] = day_ones;
  date[6] = '\0';
}

//TODO: modify the display functions so it increases based off a pre-determined months, date, etc.
// Default: Jan 1, 00:00:00
// Rewrite displayTime so it loops back after Dec 31 - 23:59:59
// Display time
void displayTime(char disp_time[9], volatile long unsigned int global_counter, volatile unsigned int adc_hour, volatile unsigned int adc_min, volatile unsigned int adc_sec) {
  unsigned int hours =  (adc_hour + (global_counter / 3600)) % 24;
  unsigned int minutes = (adc_min + (global_counter / 60)) % 60;
  unsigned int seconds = (adc_sec + global_counter) % 60;
  char hours_tens = ((hours - (hours % 10)) / 10) + '0';
  char hours_ones = (hours % 10) + '0';
  char minutes_tens = ((minutes - (minutes % 10)) / 10) + '0';
  char minutes_ones = (minutes % 10) + '0';
  char seconds_tens = ((seconds - (seconds % 10)) / 10) + '0';
  char seconds_ones = (seconds % 10) + '0';

  disp_time[0] = hours_tens;
  disp_time[1] = hours_ones;
  disp_time[2] = ':';
  disp_time[3] = minutes_tens;
  disp_time[4] = minutes_ones;
  disp_time[5] = ':';
  disp_time[6] = seconds_tens;
  disp_time[7] = seconds_ones;
  disp_time[8] = '\0';
}

// Display temp in C
void displayTempC(char disp_c[7], volatile float temperatureDegC) {
  unsigned int int_degC = (unsigned int)(temperatureDegC * 10);
  char c_tens = ((int_degC / 100) % 10) + '0';
  char c_ones = ((int_degC / 10) % 10) + '0';
  char c_tenths = (int_degC % 10) + '0';

  disp_c[0] = c_tens;
  disp_c[1] = c_ones;
  disp_c[2] = '.';
  disp_c[3] = c_tenths;
  disp_c[4] = ' ';
  disp_c[5] = 'C';
}

// Display temp in F
void displayTempF(char tempF[7], volatile float temperatureDegF) {
  unsigned int int_degF = (unsigned int)(temperatureDegF * 10);
  char f_tens = ((int_degF / 100) % 10) + '0';
  char f_ones = ((int_degF / 10) % 10) + '0';
  char f_tenths = (int_degF % 10) + '0';
  
  tempF[0] = f_tens;
  tempF[1] = f_ones;
  tempF[2] = '.';
  tempF[3] = f_tenths;
  tempF[4] = ' ';
  tempF[5] = 'F';
}


//Init User's Launchpad buttons: P2.1 (L) and P1.1 (R) (pull-up resistor) --> check for 0
void init_launchpad_button() {
  // Set digital I/O mode for pins 2.1 and 1.1
  P2SEL &= ~BIT1;
  P1SEL &= ~BIT1;
  // Set the same pins as inputs
  P2DIR &= ~BIT1;
  P1DIR &= ~BIT1;
  // Set pull resistors on the pins
  P2REN |= BIT1;
  P1REN |= BIT1;
  // Set them to be pull-up resistors
  P2OUT |= BIT1;
  P1OUT |= BIT1;
}

// Read from the User's launchpad buttons
unsigned int read_launchpad_button() {
  unsigned int pressed = 0;
  if ((~P2IN & BIT1) == BIT1) {
    pressed = 1;
  } else if ((~P1IN & BIT1) == BIT1) {
    pressed = 2;
  } else {
    pressed = 0;
  }
  return pressed;
}

// Config ADC12
void config_ADC(volatile float degC_per_bit, volatile unsigned int bits30, volatile unsigned int bits85) {
  //Set Port P8.0 (the slider) to digital I/O mode
  P8OUT |= BIT0;

  // TODO The ADC needs to read sequentially from the temp sensor, then the slider. It needs to read in from INCH_5 to a MCTL register with a VREF of 5V
  REFCTL0 &= ~REFMSTR; // Reset REFMSTR to hand over control of
  // internal reference voltages to
  // ADC12_A control registers
  ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON; // Internal ref = 1.5V
  ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1; // Enable sample timer and set sequential mode
  // Using ADC12MEM0 to store reading
  ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10; // ADC i/p ch A10 = temp sense
  // Slider stored in to MCTL1 5v reference VCC -> VSS
  ADC12MCTL1 = ADC12SREF_0 + ADC12INCH_5 + ADC12EOS;
  // ACD12SREF_1 = internal ref = 1.5v
  __delay_cycles(100); // delay to allow Ref to settle
  ADC12CTL0 |= ADC12ENC; // Enable conversion
  // Use calibration data stored in info memory (1-time setup)
  bits30 = CALADC12_15V_30C;
  bits85 = CALADC12_15V_85C;
  degC_per_bit = ((float)(85.0 - 30.0))/((float)(bits85-bits30));
}

// ADC 2 Time --> poopulate slider
volatile unsigned int ADC_2_Time(void) {
  ADC12CTL0 &= ~ADC12SC; // clear the start bit
  ADC12CTL0 |= ADC12SC + ADC12ENC; // Sampling and conversion start
  // Single conversion (single channel)
  // Poll busy bit waiting for conversion to complete
  while (ADC12CTL1 & ADC12BUSY)
    __no_operation();

  volatile unsigned int slider = ADC12MEM1; // Set store the slider value in ADC12MEM1 in slider
  return slider;
}

volatile unsigned int ADC_2_Temp(void) {
  ADC12CTL0 &= ~ADC12SC; // clear the start bit
  ADC12CTL0 |= ADC12SC + ADC12ENC; // Sampling and conversion start
  // Single conversion (single channel)
  // Poll busy bit waiting for conversion to complete
  while (ADC12CTL1 & ADC12BUSY)
    __no_operation();
  // Temp sensor stuff
  volatile unsigned int in_temp = ADC12MEM0; // Read in results if conversion
  return in_temp;
}

/***************************************************************************************************************************************** */
// Initializes the two user LEDs
void init_user_leds()
{
  //Set digital I/O mode for pins 1.0 and 4.7
  P4SEL &= ~BIT7;
  P1SEL &= ~BIT0;
  //Set the same pins as outputs
  P4DIR |= BIT7;
  P1DIR |= BIT0;
}

//Sets the two user LEDs
void set_user_leds(unsigned char uled)
{
  //zero outputs
  P4OUT &= ~BIT7;
  P1OUT &= ~BIT0;
  //set outputs
  if (uled == 1)
    P4OUT |= BIT7;
  else if (uled == 2)
    P1OUT |= BIT0;
  else if (uled == 3) {
    P4OUT |= BIT7;
    P1OUT |= BIT0;
  }
}

// Initializes the buttons for input
void init_board_buttons()
{
  // Set digital I/O mode for pins 3.6, 7.0, 7.4 and 2.2
  P3SEL &= ~BIT6;
  P7SEL &= ~(BIT4|BIT0);
  P2SEL &= ~BIT2;
  // Set the same pins as inputs
  P3DIR &= ~BIT6;
  P7DIR &= ~(BIT4|BIT0);
  P2DIR &= ~BIT2;
  // Set pull resistors on the pins
  P3REN |= BIT6;
  P7REN |= (BIT4|BIT0);
  P2REN |= BIT2;
  // Set them to be pull-up resistors
  P3OUT |= BIT6;
  P7OUT |= (BIT4|BIT0);
  P2OUT |= BIT2;
  }

//Reads the four buttons
unsigned int read_board_buttons() {
  unsigned int pressed = 0;
  pressed |= BIT0 & ~(P7IN & BIT0);
  pressed |= BIT1 & ~((P3IN & BIT6) >> 5);
  pressed |= BIT2 & ~(P2IN & BIT2);
  pressed |= BIT3 & ~((P7IN & BIT4) >> 1);
  return pressed;
}
