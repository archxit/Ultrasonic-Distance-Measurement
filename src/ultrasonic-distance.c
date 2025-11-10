/***************************************************************
 * Project      : Ultrasonic Distance Measurement System
 * Microcontroller : NXP LPC1768 (ARM Cortex-M3)
 * Peripherals  : HC-SR04 Ultrasonic Sensor, 16x2 LCD (4-bit mode),
 *                LEDs on P0.4-P0.11, Buzzer on P0.22
 * Author       : Archit, Sridha, Maaheer
 * Description  :
 *   - Measures distance using ultrasonic sensor (TRIG P0.15, ECHO P0.16)
 *   - Displays distance on 16x2 LCD
 *   - Turns ON LEDs & buzzer if object < 10 cm
 *   - Turns ON LEDs if object < 20 cm
 ****************************************************************/


#include <stdio.h>
#include <LPC17xx.h>
#include <string.h>


// ---------------- PIN DEFINITIONS ----------------
#define LED_Pinsel     0xFF        // LEDs on P0.4 - P0.11
#define TRIGGER_PIN    (1 << 15)   // Ultrasonic Trigger Pin (Output)
#define ECHO_PIN       (1 << 16)   // Ultrasonic Echo Pin (Input)


// ---------------- GLOBAL VARIABLES ----------------
char ans[20] = "";       // LCD string buffer
int temp, temp1, temp2 = 0;
int flag = 0, flag_command = 0;
int i, j, k, l, r;
int echoTime = 5000;     // Time duration of echo pulse
float distance = 0;      // Calculated distance in cm


// ---------------- FUNCTION DECLARATIONS ----------------
void lcd_wr(void);
void port_wr(void);
void delay(int r1);
void timer_start(void);
float timer_stop(void);
void timer_init(void);
void dealy_in_US(unsigned int microseconds);
void dealy_in_MS(unsigned int milliseconds);


// --------------------------------------------------------
// MICROSECOND DELAY USING TIMER0
// --------------------------------------------------------
void dealy_in_US(unsigned int microseconds){
    LPC_TIM0->TCR = 0x02;            // Reset Timer
    LPC_TIM0->PR = 0;                // Prescaler = 0
    LPC_TIM0->MR0 = microseconds - 1; // Match value
    LPC_TIM0->MCR = 0x01;            // Interrupt on match
    LPC_TIM0->TCR = 0x01;            // Enable timer


    while ((LPC_TIM0->IR & 0x01) == 0); // Wait for match flag


    LPC_TIM0->TCR = 0x00;            // Stop timer
    LPC_TIM0->IR = 0x01;             // Clear interrupt flag
}


void dealy_in_MS(unsigned int milliseconds){
    dealy_in_US(milliseconds * 1000);
}


// --------------------------------------------------------
// TIMER INITIALIZATION FOR ECHO MEASUREMENT
// --------------------------------------------------------
void timer_init(void){
    LPC_TIM0->CTCR = 0x0;            // Timer mode
    LPC_TIM0->PR = 11999999;         // 12 MHz clock division
    LPC_TIM0->TCR = 0x02;            // Reset timer
}


void timer_start(void){
    LPC_TIM0->TCR = 0x02;            // Reset timer
    LPC_TIM0->TCR = 0x01;            // Start timer
}


float timer_stop(void){
    LPC_TIM0->TCR = 0x00;            // Stop timer
    return LPC_TIM0->TC;             // Return captured value
}


// --------------------------------------------------------
// LEVEL DELAY FUNCTION (SOFTWARE)
// --------------------------------------------------------
void delay(int r1){
    for (r = 0; r < r1; r++);
}


// --------------------------------------------------------
// LCD LOW-LEVEL WRITE (4-BIT INTERFACE)
// --------------------------------------------------------
void port_wr(void){
    LPC_GPIO0->FIOPIN = temp2 << 23;   // Data on P0.23-P0.26


    if (flag_command == 0)
        LPC_GPIO0->FIOCLR = 1 << 27;   // RS = 0 → command
    else
        LPC_GPIO0->FIOSET = 1 << 27;   // RS = 1 → data


    LPC_GPIO0->FIOSET = 1 << 28;       // EN = 1 (Latch)
    for (j = 0; j < 50; j++);
    LPC_GPIO0->FIOCLR = 1 << 28;       // EN = 0
    for (j = 0; j < 10000; j++);
}


void lcd_wr(void){
    temp2 = (temp1 >> 4) & 0xF;  // Send upper nibble
    port_wr();
    temp2 = temp1 & 0xF;         // Send lower nibble
    port_wr();
}


// --------------------------------------------------------
// MAIN PROGRAM
// --------------------------------------------------------
int main(){
    int command_init[] = {3,3,3,2,2,0x01,0x06,0x0C,0x80};  // LCD init sequence


    SystemInit();
    SystemCoreClockUpdate();
    timer_init();


    // GPIO CONFIGURATION
    LPC_PINCON->PINSEL0 &= 0xfffff00f;    // LEDs P0.4-P0.11
    LPC_PINCON->PINSEL0 &= 0x3fffffff;    // TRIGGER P0.15 GPIO
    LPC_PINCON->PINSEL1 &= 0xfffffff0;    // ECHO P0.16 GPIO


    LPC_GPIO0->FIODIR |= TRIGGER_PIN | (1<<22);    // Trigger + Buzzer output
    LPC_GPIO1->FIODIR |= 0 << 16;                  // Echo input
    LPC_GPIO0->FIODIR |= LED_Pinsel << 4;          // LEDs output
    LPC_GPIO0->FIODIR |= 0xF << 23 | 1<<27 | 1<<28;// LCD pins


    // LCD INITIALIZATION
    flag_command = 0;
    for (i = 0; i < 9; i++){
        temp1 = command_init[i];
        lcd_wr();
        for (j = 0; j < 30000; j++);
    }


    while (1){
        // SEND 10us TRIGGER PULSE
        LPC_GPIO0->FIOSET |= TRIGGER_PIN;
        dealy_in_US(10);
        LPC_GPIO0->FIOCLR |= TRIGGER_PIN;


        // WAIT FOR ECHO HIGH
        while (!(LPC_GPIO0->FIOPIN & ECHO_PIN));


        timer_start();   // START TIMER


        // WAIT UNTIL ECHO GOES LOW
        while (LPC_GPIO0->FIOPIN & ECHO_PIN);


        echoTime = timer_stop();    // READ TIMER VALUE


        distance = (0.00343 * echoTime) / 2;  // Convert to cm


        sprintf(ans, " Distance: %.3f", distance);


        // CLEAR LCD AND DISPLAY RESULT
        flag_command = 0;
        temp1 = 0x01;     
        lcd_wr();


        flag_command = 1;
        for (i = 0; ans[i] != '\0'; i++){
            temp1 = ans[i];
            lcd_wr();
            for (j = 0; j < 300; j++);
        }
		dealy_in_MS(100);

        // ALERT IF DISTANCE TOO SMALL
	    if(distance <20)
		{
            LPC_GPIO0->FIOSET = LED_Pinsel << 4;  // LEDs ON
            if (distance < 10)
            {
                LPC_GPIO0->FIOSET = 1 << 22;          // Buzzer ON
            }
            else
            {
                LPC_GPIO0->FIOCLR = 1 << 22;          // Buzzer OFF
            }
			
		}
        else {
            LPC_GPIO0->FIOCLR = LED_Pinsel << 4;  // LEDs OFF
            LPC_GPIO0->FIOCLR = 1 << 22;          // Buzzer OFF
        }


        dealy_in_MS(5000);
    }
}
