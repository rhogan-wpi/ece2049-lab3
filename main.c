#include "functions.h"

// Globals
volatile unsigned int in_temp = 0;
volatile unsigned int slider = 0;
volatile long unsigned int global_counter = 16416000;
volatile float temperatureDegC = 0;
volatile float temperatureDegF = 0;
volatile float degC_per_bit = 0;
volatile unsigned int adc_month = 1;
volatile unsigned int adc_date = 1;
volatile unsigned int adc_hour = 0;
volatile unsigned int adc_min = 0;
volatile unsigned int adc_sec = 0;
volatile unsigned int bits30, bits85;
volatile state mode;

#pragma vector=TIMER2_A0_VECTOR //What does this do? No one knows...
__interrupt void timer_a2() {
  global_counter++;
  // Graphics_clearDisplay(&g_sContext); // Clear the display
  // Graphics_flushBuffer(&g_sContext);
}

void main() {
  WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer. Always need to stop this!!

  _BIS_SR(GIE); // Global interrupt enable

  // Initialize the MSP430
  // configKeypad();
  // initLeds();
  // init_user_leds();
  // init_board_buttons();
  // config_ADC(degC_per_bit, bits30, bits85); // Config the ADC12

  /*************************Testing ONLY******************************** */
  //Set up Port P8.0 (the slider) to digital I/O mode
  P8SEL &= ~BIT0;
  P8DIR |= BIT0;
  P8OUT |= BIT0;
  // TODO The ADC needs to read sequentially from the temp sensor, then the slider. It needs to read in from INCH_5 to a MCTL register with a VREF of 5V
  REFCTL0 &= ~REFMSTR; // Reset REFMSTR to hand over control of
  // internal reference voltages to
  // ADC12_A control registers
  ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON | ADC12MSC; // Internal ref = 1.5V
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
  /**************************************************************************** */
  
  configDisplay();
  Graphics_clearDisplay(&g_sContext); // Clear the display
  runtimerA2(); // Start the A2 timer
  // Array for display functions.
  char date[7] = {0};
  char time[9] = {0};
  char tempC_disp[7] = {0};
  char tempF_disp[7] = {0};
  // Array for the Moving Average
  float val_tempC[MOVING_AVERAGE_SIZE] = {0.0};
  float val_tempF[MOVING_AVERAGE_SIZE] = {0.0};
  float sum_tempC = 0.0;
  float sum_tempF = 0.0;
  unsigned int index = 0;

  mode = DISPLAY; // Main  mode
  unsigned int user_input = read_launchpad_button(); // Read the User's Push-buttons

  while (1) {
    switch(mode) {
      case DISPLAY: {
        while(user_input != 1) { // Left button
          in_temp = ADC_2_Temp(); // ADC Conversion stuff:populate in_temp
          temperatureDegC = (float)((long)in_temp - CALADC12_15V_30C) * degC_per_bit +30.0;
          temperatureDegF = temperatureDegC * 9.0/5.0 + 32.0; // Temperature in Fahrenheit Tf = (9/5)*Tc + 32
          index = global_counter % SIZE;
          
          // Moving-average logic
          sum_tempC -= tempC[index]; // Remove the oldest readings
          tempC[index] = temperatureDegC;
          sum_tempC += temperatureDegC; // Add the newest readings

          sum_tempF -= tempF[index];
          tempF[index] = temperatureDegF;
          sum_tempF += temperatureDegF;
          
          // Display stuff
          Graphics_clearDisplay(&g_sContext);
          displayDate(date, global_counter, adc_month, adc_date);
          Graphics_drawStringCentered(&g_sContext, date, 7, 48, 15, TRANSPARENT_TEXT);
          
          displayTime(time, global_counter, adc_hour, adc_min, adc_sec);
          Graphics_drawStringCentered(&g_sContext, time, 9, 48, 35, TRANSPARENT_TEXT);
          
          displayTempC(tempC, (sum_tempC / MOVING_AVERAGE_SIZE));
          Graphics_drawStringCentered(&g_sContext, tempC_disp, 6, 48, 45, TRANSPARENT_TEXT);
          
          displayTempF(tempF, (sum_tempF / MOVING_AVERAGE_SIZE));
          Graphics_drawStringCentered(&g_sContext, tempF_disp, 6, 48, 55, TRANSPARENT_TEXT);
          Graphics_flushBuffer(&g_sContext);
        }

        mode = EDIT;
        break;
      }

      case EDIT: {
        unsigned int num_pressed = 0;
        while (user_input != 2) { // Right button
          slider = ADC_2_Time(); // ADC Conversion stuff: populate slider
          num_pressed += (read_launchpad_button() % 5); // Wrap around to "Month" logic
          // Traversing logic
          switch (num_pressed) {
            case 0: { //MONTH
              adc_month = 1 + (volatile unsigned int)(slider / ONE_MONTH_IN_ADC);
              Graphics_clearDisplay(&g_sContext);
              displayDate(date, 0, adc_month, adc_date); // "Date" has not been updated yet.
              Graphics_flushBuffer(&g_sContext);
              break;    
            }

            case 1: { // DATE
              if (adc_month == 2) {
                adc_date = 1 + (volatile unsigned int)(slider / 147);
              } else if ((adc_month == 4) &&  (adc_month == 6) && (adc_month == 9) && (adc_month == 11)) {
                adc_date = 1 + (volatile unsigned int)(slider / 137);
              } else {
                adc_date = 1 + (volatile unsigned int)(slider / 133);
              } 
              Graphics_clearDisplay(&g_sContext);
              displayDate(date, 0, adc_month, adc_date); // "Month" and "Date" have been updated
              Graphics_flushBuffer(&g_sContext); 
              break;   
            }

            case 2: { // HOUR
              adc_hour = (volatile unsigned int)(slider / 171);    
              Graphics_clearDisplay(&g_sContext);
              displayTime(time, 0, adc_hour, adc_min, adc_sec); // "Min" and "Sec" have not been updated --> use the previous values stored.
              Graphics_flushBuffer(&g_sContext); 
              break;
            }

            case 3: { // MIN
              adc_min = (volatile unsigned int)(slider / 69);
              Graphics_clearDisplay(&g_sContext);
              displayTime(time, 0, adc_hour, adc_min, adc_sec); // "Hour" has been updated. "Sec" has not been updated
              Graphics_flushBuffer(&g_sContext); 
              break;
            }

            case 4: { // SEC
              adc_sec = (volatile unsigned int)(slider / 69);
              Graphics_clearDisplay(&g_sContext);
              displayTime(time, 0, adc_hour, adc_min, adc_sec); // Every param has been updated.
              Graphics_flushBuffer(&g_sContext);
              break;    
            }
          } // End of switch num_pressed
        } // End of while loop

        mode = DISPLAY;
        break;
      } // End of case EDIT
    }  // End of switch mode
  } // End of while(1)
} // End of main()
