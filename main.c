/* ==============================================================================================
	
	Programa: 
        FSM Semaphore PIC12F683
	
	Descricao: 
        Implementacao de uma maquina de estados para controle de um semaforo utilizando
        biblioteca HAL.
	
	Autor:           Eng. Jonnes Nascimento
	Local, Data:     Santos, 16/12/2021
	Data de Revisao: 29/12/2021
	
================================================================================================*/

/* ==============================================================================================
    Configuracoes
================================================================================================*/
#if defined (__XC8__)
    #pragma config FOSC     = INTOSCCLK // Oscillator Selection bits (INTOSC oscillator: CLKOUT function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
    #pragma config WDTE     = OFF       // Watchdog Timer Enable bit (WDT disabled)
    #pragma config PWRTE    = OFF       // Power-up Timer Enable bit (PWRT disabled)
    #pragma config MCLRE    = ON       // MCLR Pin Function Select bit (MCLR pin function is disabled)
    #pragma config CP       = OFF       // Code Protection bit (Program memory code protection is disabled)
    #pragma config CPD      = OFF       // Data Code Protection bit (Data memory code protection is disabled)
    #pragma config BOREN    = OFF       // Brown Out Detect (BOR enabled)
    #pragma config IESO     = OFF       // Internal External Switchover bit (Internal External Switchover mode is enabled)
    #pragma config FCMEN    = OFF       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#else
    #error Configs not defined due compiler is not foreseen
#endif

/* ==============================================================================================
	Bibliotecas 
================================================================================================*/
#include <xc.h>
#include "gpio.h"

/* ==============================================================================================
	Definicoes de Tipos de Dados e Estruturas
================================================================================================*/
#define _XTAL_FREQ 8000000


/* ==============================================================================================
	Constantes e Definicoes
================================================================================================*/


/* ==============================================================================================
	Variaveis Globais
================================================================================================*/
uint8_t fg_blink = 1;
//unsigned char count;

/* ==============================================================================================
	Prototipos de Funcoes Auxiliares
================================================================================================*/
void delay_ms(uint16_t tempo);
void f_toggle_led(void);


/* ==============================================================================================
	Funcao Principal
================================================================================================*/
void main(void) 
{
    // register all ISR callback functions
    Gpio_Callback_Register(GPIO_EXTERNAL_INTERRUPT, f_toggle_led);
    
    const GpioConfig_t * config = GpioConfigGet();
    Gpio_Init(config);
    
    Gpio_Write(CHANNEL_GP0, GPIO_PIN_STATE_HIGH);
    
    while (1)
    {
        if (fg_blink)
       {
            Gpio_Toggle(CHANNEL_GP0);
            delay_ms(500);
       }

    }
} /* main */


/* ==============================================================================================
	Funcoes Auxiliares
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
	Fim do Programa
================================================================================================*/

