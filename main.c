/************************************************************************************************
 * @Title          :    Main function
 * @Filename       :    main.c
 * @Author         :    Eng. Jonnes F. Nascimento
 * @Origin Date    :    16/12/2021
 * @Version        :    0.0.1
 * @Compiler       :    XC8
 * @Target         :    PIC12F683
 * @Notes          :    None
 * @Revision Date  :    30/12/2021
 ************************************************************************************************/
/** **********************************************************************************************
 * @file main.c
 * @brief Main program
 *
 *  This is the entrypoint of the firmware.
 *
 *! \mainpage
 *
 * \section intro_sec Project Introduction
 * Microcontroller HAL Reference Implementation <br />
 *
 * *** Work in Progress *** <br />
 *
 * This work has been written as a basis for the study of microcontrollers drivers and is based mainly on Jacob Beningo's book <a target="_blank" href="https://www.amazon.com/Reusable-Firmware-Development-Practical-Approach/dp/1484232968">Reusable Firmware Development</a>. Apress. 2017, and Elecia White's book <a target="_blank" href="https://www.amazon.com/Making-Embedded-Systems-Patterns-Software/dp/1449302149">Making Embedded Systems</a>. O'Reilly. 2011.
 *
 * Author: Eng. Jonnes F. Nascimento. 2021
 *
 * \section ver_sec Version Log
 * \section acron_sec Acronyms
 * \section soft_sec Software Architecture Overview
 * \section api_sec APIs
 * \section hal_sec HALs
 * \section mid_sec Middleware
 * \section os_sec OS Information
 * \section std_sec Coding Standards
 * \section doc_seq Documentation
 * \section req_sec Project Requirements
 * \section test_sec Testing and Validation
 * <br>
 * \section tools Tools
 * \subsection compiler Compiler
 * GNU C Compiler (GCC)
 *
 */


/* ==============================================================================================
    Configurations
================================================================================================*/
#if defined (__XC8__)
    #pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
    #pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
    #pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
    #pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
    #pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
    #pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
    #pragma config BOREN = OFF      // Brown Out Detect (BOR disabled)
    #pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
    #pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#else
    #error Configs not defined due compiler is not foreseen
#endif


/* ==============================================================================================
	Libraries 
================================================================================================*/
#include <xc.h>
#include "hal/drivers/gpio/gpio.h"


/* ==============================================================================================
	Defines and Macros
================================================================================================*/
#define _XTAL_FREQ 8000000


/* ==============================================================================================
	Constants
================================================================================================*/


/* ==============================================================================================
	Global Variables
================================================================================================*/
uint8_t fg_blink = 1;
//unsigned char count;
GpioPinState_t pin_4;
GpioPort_t gpio_v;


/* ==============================================================================================
	Prototypes
================================================================================================*/
void delay_ms(uint16_t tempo);
void f_toggle_led(void);


/* ==============================================================================================
	Main Function
================================================================================================*/
void main(void) 
{
    // register all ISR callback functions
    Gpio_Callback_Register(GPIO_EXTERNAL_INTERRUPT, f_toggle_led);
    
    const GpioConfig_t * config = GpioConfigGet();
    Gpio_Init(config);
    
    Gpio_Write(CHANNEL_GP4, GPIO_PIN_STATE_HIGH);
    
    pin_4 = Gpio_Read(CHANNEL_GP4);
        
    gpio_v.BIT0 = 1;
    gpio_v.BIT1 = 1;
    gpio_v.BIT2 = 1;
    gpio_v.BIT3 = 1;
    gpio_v.BIT4 = 1;
    gpio_v.BIT5 = 1;
    
    Gpio_Register_Write(gpio_v);
    
    gpio_v = Gpio_Register_Read();
    
    while (1)
    {
       if (fg_blink)
       {
           Gpio_Toggle(CHANNEL_GP0);
           delay_ms(500);
       }
       else
       {           
           Gpio_Write(CHANNEL_GP0, GPIO_PIN_STATE_LOW); // always turns off the led pin (GP0)
       }
    }
} /* main */


/* ==============================================================================================
	Auxiliary Functions
================================================================================================*/
void delay_ms(uint16_t tempo)
{
    while (tempo > 0)
    {
        __delay_ms(1);
        tempo--;
    }
}


void f_toggle_led()
{
    fg_blink ^= 1;
}

/* ==============================================================================================
	EOF
================================================================================================*/

