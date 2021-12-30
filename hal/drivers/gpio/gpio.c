/************************************************************************************************
 * @Title          :    General purpose input/output (GPIO)
 * @Filename       :    gpio.c
 * @Author         :    Eng. Jonnes F. Nascimento
 * @Origin Date    :    16/12/2021
 * @Version        :    0.0.1
 * @Compiler       :    XC8
 * @Target         :    PIC12F683
 * @Notes          :    None
 * @Revision Date  :    30/12/2021
 ************************************************************************************************/
/** **********************************************************************************************
 * @file gpio.c
 * @brief Interface implementation for general purpose input/output (GPIO)
 *
 *  This is the implementation file for the interface of a digital
 *  input / output peripheral on a standard microcontroller. Some microcontrollers
 *  call this digital I/O as PORT_x. In this case, this is called GPIO.
 *
 * @addtogroup Drivers
 * Peripheral drivers group (GPIO, TIMERS, SPI, etc).
 *  @{
 * */

/************************************************************************************************
 * Includes
 ************************************************************************************************/
#include <stdio.h>
#include "gpio.h"
#include "../hardware/pic12f683.h"
#include "../../library/constants.h"

/************************************************************************************************
 * Defines and Macros
 ************************************************************************************************/

/************************************************************************************************
 * Static Typedefs
 ************************************************************************************************/
/**
 * Define a helper type to store the event's callback function
 * */
typedef struct
{
    GpioCallbackEvent_t Event;          /**< Event that triggers the interruption */
    void (*CallbackFunction)(void);     /**< Function to callback on interruption */
} CallbackRegister_t;

/************************************************************************************************
 * Global Variables
 ************************************************************************************************/
//extern unsigned char count;

/************************************************************************************************
 * Module Variables Definitions
 ************************************************************************************************/
/**
 * List to store the events' callback functions
 * */
static CallbackRegister_t CallbackRegisterList[GPIO_CALLBACK_EVENT_COUNT];

/* This is for test only - this code must be moved */
static ARCH_ADDR_SIZE volatile * const SystemOscilator = {
        (ARCH_ADDR_SIZE *) OSCCON
};

static ARCH_ADDR_SIZE volatile * const SystemComparator = {
        (ARCH_ADDR_SIZE *) CMCON0
};

static ARCH_ADDR_SIZE volatile * const SystemADConverter = {
        (ARCH_ADDR_SIZE *) ADCON0
};
/* *************************************************************************/

static ARCH_ADDR_SIZE volatile * const GpioPort[GPIO_PORT_COUNT] = {
        (ARCH_ADDR_SIZE *) GPIO
};

static ARCH_ADDR_SIZE volatile * const GpioDirection[GPIO_PORT_COUNT] = {
        (ARCH_ADDR_SIZE *) TRISIO
};

static ARCH_ADDR_SIZE volatile * const GpioFunction[GPIO_PORT_COUNT] = {
        (ARCH_ADDR_SIZE *) ANSEL
};

static ARCH_ADDR_SIZE volatile * const InterruptControl[GPIO_PORT_COUNT] = {
        (ARCH_ADDR_SIZE *) INTCON
};

static ARCH_ADDR_SIZE volatile * const GpioOption[GPIO_PORT_COUNT] = {
        (ARCH_ADDR_SIZE *) OPTION
};

static ARCH_ADDR_SIZE volatile * const PeripheralInterruptRequest[GPIO_PORT_COUNT] = {
        (ARCH_ADDR_SIZE *) PIR1
};

static ARCH_ADDR_SIZE volatile * const PeripheralInterruptEnable[GPIO_PORT_COUNT] = {
        (ARCH_ADDR_SIZE *) PIE1
};
/************************************************************************************************
 * Functions Prototypes
 ************************************************************************************************/
static void Gpio_Interrupt_Callback_Executor(void);

/************************************************************************************************
 * Functions Definitions
 ************************************************************************************************/

/************************************************************************************************
 * Function: void Gpio_Init(const GpioConfig_t * const Config);
 *//**
 *
 * \b Description:
 *
 * This function is used to initializes the GPIO port based on the
 * configuration table defined in gpio_config module.
 *
 * Pre-condition: Configuration table defined (sizeof > 0) <br>
 * Pre-condition: NUMBER_OF_PORTS > 0 <br>
 * Pre-condition: GPIO_MAX_PIN_NUMBER > 0 <br>
 * Pre-condition: System's oscillator, comparator and A/D converters initialized <br>
 * Pre-condition: All interrupts' callback functions registered (calling Gpio_Callback_Register function) <br>
 *
 * Post-condition: GPIO port initialized and configured
 *
 * @param [in]          Config is a pointer to configuration table for
 *                      the GPIO peripheral.
 *
 * @return              void
 *
 * \b Example:
 *
 * @code
 * Gpio_Init(GpioConfigGet());
 * @endcode
 *
 * @see Gpio_Write
 * @see Gpio_Read
 * @see Gpio_Toggle
 * @see Gpio_Register_Write
 * @see Gpio_Register_Read
 * @see Gpio_Callback_Register
 *
 * <b>HISTORY OF CHANGES</b>
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 16/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Creation Date </td></tr>
 * <tr><td> 17/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Documentation Created </td></tr>
 * <tr><td> 30/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Small adjustments </td></tr>
 * </table><br><br><hr>
 ************************************************************************************************/
void Gpio_Init(const GpioConfig_t * const Config)
{
    /* This is for test only - this code must be moved */
    *SystemOscilator    = 0x70; /**< Switch to 8 MHz system clock */
    *SystemComparator   = 0x07; /**< Comparators off */
    *SystemADConverter  = 0x00; /**< AD Converters off */
            
    // Disable all interrupts
    *InterruptControl[GPIO_PORT]           = 0x00;
    *PeripheralInterruptEnable[GPIO_PORT]  = 0x00;
    *PeripheralInterruptRequest[GPIO_PORT] = 0x00;
    
    for (uint8_t port = 0; port < GPIO_PORT_COUNT; port++)
    {
        *GpioPort[port]     = 0x00; /**< Resets all GPIO pins */
        *GpioFunction[port] = 0x00; /**< ANSEL: digital pins as I/O */
                
        /* Sets the GPIOs */
        for (uint8_t i = 0; i < GPIO_CHANNEL_COUNT; i++)
        {
           switch (Config[i].Function)
           {
               case GPIO_PIN_FUNCTION_DIGITAL:
                   // TRISIO: set de pin direction mode
                   if (Config[i].Direction == GPIO_PIN_DIRECTION_INPUT)
                   {
                       *GpioDirection[port] |= (Config[i].Direction << i);
                   }
                   else
                   {
                       *GpioDirection[port] &= ~(HIGH << i);
                   }
                   break;
               case GPIO_PIN_FUNCTION_CAPTURE_COMPARE: break;
               case GPIO_PIN_FUNCTION_COMPARATOR: break;
               case GPIO_PIN_FUNCTION_INTERRUPT_ON_CHANGE: break;
               case GPIO_PIN_FUNCTION_TIMER: break;
               case GPIO_PIN_FUNCTION_ANALOG:
                   *GpioFunction[port] |= (HIGH << i);
                   break;                   
               case GPIO_PIN_FUNCTION_EXTERNAL_INTERRUPT:
                   // ANSEL as digital I/O
                   *GpioFunction[port] &= ~(HIGH << i);
                   
                   // TRISIO set de direction of pin as INPUT
                   *GpioDirection[port] |= (Config[i].Direction << i);

                   // Sets the edge trigger of the interrupt
                   if (Config[i].InterruptEdge == GPIO_INTERRUPT_EDGE_FALLING)
                   {
                       *GpioOption[port] &= ~INTEDG_BIT_MASK;
                   }
                   else if (Config[i].InterruptEdge == GPIO_INTERRUPT_EDGE_RISING)
                   {
                       *GpioOption[port] |= INTEDG_BIT_MASK;
                   }
                   else
                   {
                       // do nothing
                   }

                   // Clear the external interrupt flag INTF
                   *InterruptControl[port] &= ~INTF_BIT_MASK;

                   // Enable external interrupts
                   *InterruptControl[port] |= INTE_BIT_MASK;

                   // Enable global interrupts
                   *InterruptControl[port] |= GIE1_BIT_MASK;

                   // Save the callback function executor address to the FSR register
                   //*FSR = ?
                   //asm volatile ("JMP *%0" : : "r" (&Gpio_Interrupt_Callback_Executor));
                   
                   break;
           } // switch Config[i].Function
           
           // sets the GPIO initial state
           *GpioPort[port] |= (Config[i].State << i);
           
        } // for GPIO_CHANNEL_COUNT
    } // for GPIO_PORT_COUNT
} // Gpio_Init

/************************************************************************************************
 * Function: void Gpio_Write(GpioChannel_t channel, GpioPinState_t state);
 *//**
 *
 * \b Description:
 *
 * This function is used to write a bit, specified by a channel, to GPIO port.
 *
 * Pre-condition: GPIO port must have been initialized <br>
 *
 * Post-condition: GPIO port bit setted or cleared
 *
 * @param [in]          channel is a GpioChannel_t that represents a bit on GPIO port
 * @param [in]          state is a GpioPinState_t that represents a state of the bit on GPIO port
 *
 * @return              void
 *
 * \b Example:
 *
 * @code
 * Gpio_Write(CHANNEL_GP3, GPIO_PIN_STATE_HIGH);
 * @endcode
 *
 * @see Gpio_Init
 * @see Gpio_Read
 * @see Gpio_Toggle
 * @see Gpio_Register_Write
 * @see Gpio_Register_Read
 * @see Gpio_Callback_Register
 *
 * <b>HISTORY OF CHANGES</b>
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 16/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Creation Date </td></tr>
 * <tr><td> 17/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Documentation Created </td></tr>
 * <tr><td> 30/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Small adjustments </td></tr>
 * </table><br><br><hr>
 ************************************************************************************************/
void Gpio_Write(GpioChannel_t channel, GpioPinState_t state)
{
    if (state == GPIO_PIN_STATE_HIGH)
    {
        *GpioPort[GPIO_PORT] |= (HIGH << channel);
    }
    else
    {
        *GpioPort[GPIO_PORT] &= ~(HIGH << channel);
    }
}

/************************************************************************************************
 * Function: GpioPinState_t Gpio_Read(GpioChannel_t channel);
 *//**
 *
 * \b Description:
 *
 * This function is used to read a pin state, specified by a channel, from GPIO port.
 *
 * Pre-condition: GPIO port must have been initialized <br>
 *
 * Post-condition: GPIO port state is returned
 *
 * @param [in]          channel is a GpioChannel_t that represents a bit on GPIO port
 *
 * @return              GpioPinState_t
 *
 * \b Example:
 *
 * @code
 * GpioPinState_t PinState = Gpio_Read(CHANNEL_GP3);
 * @endcode
 *
 * @see Gpio_Init
 * @see Gpio_Write
 * @see Gpio_Toggle
 * @see Gpio_Register_Write
 * @see Gpio_Register_Read
 * @see Gpio_Callback_Register
 *
 * <b>HISTORY OF CHANGES</b>
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 20/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Creation Date </td></tr>
 * </table><br><br><hr>
 ************************************************************************************************/
GpioPinState_t Gpio_Read(GpioChannel_t channel)
{
    return ((*GpioPort[GPIO_PORT] & (HIGH << channel)) >= 1 ? GPIO_PIN_STATE_HIGH : GPIO_PIN_STATE_LOW);
}

/************************************************************************************************
 * Function: void Gpio_Toggle(GpioChannel_t channel);
 *//**
 *
 * \b Description:
 *
 * This function is used to toggle a pin state, specified by a channel, from GPIO port.
 *
 * Pre-condition: GPIO port must have been initialized <br>
 *
 * Post-condition: GPIO port state is toggled
 *
 * @param [in]          channel is a GpioChannel_t that represents a bit on GPIO port
 *
 * @return              void
 *
 * \b Example:
 *
 * @code
 * Gpio_Toggle(CHANNEL_GP3);
 * @endcode
 *
 * @see Gpio_Init
 * @see Gpio_Write
 * @see Gpio_Read
 * @see Gpio_Register_Write
 * @see Gpio_Register_Read
 * @see Gpio_Callback_Register
 *
 * <b>HISTORY OF CHANGES</b>
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 20/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Creation Date </td></tr>
 * </table><br><br><hr>
 ************************************************************************************************/
void Gpio_Toggle(GpioChannel_t channel)
{
    *GpioPort[GPIO_PORT] ^= (HIGH << channel);
}

/************************************************************************************************
 * Function: void Gpio_Register_Write(GpioPort_t Port);
 *//**
 *
 * \b Description:
 *
 * This function is used to write a byte that represents all GPIO port bits to GPIO port.
 *
 * Pre-condition: GPIO port must have been initialized <br>
 *
 * Post-condition: All GPIO port bits sets according to the byte parameter
 *
 * @param [in]          Port is a GpioPort_t that represents all GPIO bits
 *
 * @return              void
 *
 * \b Example:
 *
 * @code
 * Gpio_Register_Write(0x04);
 * @endcode
 *
 * @see Gpio_Init
 * @see Gpio_Write
 * @see Gpio_Read
 * @see Gpio_Toggle
 * @see Gpio_Register_Read
 * @see Gpio_Callback_Register
 *
 * <b>HISTORY OF CHANGES</b>
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 16/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Creation Date </td></tr>
 * <tr><td> 17/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Documentation Created </td></tr>
 * </table><br><br><hr>
 ************************************************************************************************/
void Gpio_Register_Write(GpioPort_t Port)
{
    *GpioPort[GPIO_PORT] = Port.BYTE;
}

/************************************************************************************************
 * Function: GpioPort_t Gpio_Register_Read(void);
 *//**
 *
 * \b Description:
 *
 * This function is used to read GPIO port byte.
 *
 * Pre-condition: GPIO port must have been initialized <br>
 *
 * Post-condition: None
 *
 * @param [in]          void
 *
 * @return              GpioPort_t
 *
 * \b Example:
 *
 * @code
 * GpioPort_t Port = Gpio_Register_Read();
 * @endcode
 *
 * @see Gpio_Init
 * @see Gpio_Write
 * @see Gpio_Read
 * @see Gpio_Toggle
 * @see Gpio_Register_Write
 * @see Gpio_Callback_Register
 *
 * <b>HISTORY OF CHANGES</b>
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 20/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Creation Date </td></tr>
 * </table><br><br><hr>
 ************************************************************************************************/
volatile GpioPort_t Gpio_Register_Read(void)
{
    return (GpioPort_t) *GpioPort[GPIO_PORT];
}

/************************************************************************************************
 * Function: void Gpio_Callback_Register(GpioCallbackEvent_t event, void (*CallbackFunction)(void));
 *//**
 *
 * \b Description:
 *
 * This function is used to register a callback function to an interrupt event
 *
 * Pre-condition: GPIO port must have been initialized <br>
 * Pre-condition: The GPIO port bit must have been configured to trigger the interrupt
 *
 * Post-condition: The callback function is called to handles the interrupt event
 *
 * @param [in]          event is a pre-configured interrupt event
 * @param [in]          (*CallbackFunction)(void) is the callback function to call
 *
 * @return              void
 *
 * \b Example:
 *
 * @code
 * Gpio_Callback_Register(GPIO_EXTERNAL_INTERRUPT, BlinkLED);
 * @endcode
 *
 * @see Gpio_Init
 * @see Gpio_Write
 * @see Gpio_Read
 * @see Gpio_Toggle
 * @see Gpio_Register_Write
 * @see Gpio_Register_Read
 *
 * <b>HISTORY OF CHANGES</b>
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 20/12/2021 </td><td> 0.0.1            </td><td> JFN      </td><td> Creation Date </td></tr>
 * </table><br><br><hr>
 ************************************************************************************************/
void Gpio_Callback_Register(GpioCallbackEvent_t Event, void (*CallbackFunction)(void))
{
    CallbackRegister_t CallbackFunctionReg = {.Event = Event, .CallbackFunction = CallbackFunction};
    CallbackRegisterList[Event] = CallbackFunctionReg;
}

/************************************************************************************************
 * Auxiliary Functions
 ************************************************************************************************/
static void __interrupt() Gpio_Interrupt_Callback_Executor(void)
{
    // Disable external interrupts (INTCON.INTE)

    // Save W register

    // Save STATUS register

    // Verify interrupts flags to determine which callback function to call

    /*
      if (INTCON.INTF)
      {
           CallbackRegisterList[GPIO_EXTERNAL_INTERRUPT].CallbackFunction();
     
           // Clear interrupt flag
      }
     */
    //

    // Restore W register

    // Restore STATUS register

    // Enable external interrupts (INTCON.INTE)

    // Enable global interrupts
    
    //count++;
    
    CallbackRegisterList[GPIO_EXTERNAL_INTERRUPT].CallbackFunction();
    
    *InterruptControl[GPIO_PORT] &= ~INTF_BIT_MASK;
}

/************************************************************************************************
 * EOF
 ************************************************************************************************/
/** @} */