/*
 * ADC Y DAC
 * */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "driverlib/rom.h"
#include "driverlib/fpu.h"
//
//*****************************************************************************

//*****************************************************************************
// Definiciones para configuraci�n del SPI y variable global
//*****************************************************************************
#define NUM_SPI_DATA    1  // N�mero de palabras que se env�an cada vez
#define VALOR_MAX    4095  // Valor m�ximo del contador
#define SPI_FREC  4000000  // Frecuencia para el reloj del SPI
#define SPI_ANCHO      16  // N�mero de bits que se env�an cada vez, entre 4 y 16

int valor_ADC = 0;
uint16_t cont = 0;

//*****************************************************************************
// The interrupt handler for the timer interrupt.
//*****************************************************************************
void
Timer0IntHandler(void)
{
    uint32_t pui32DataTx[NUM_SPI_DATA]; // la funci�n put pide tipo uint32_t
    uint8_t ui32Index;

    // Clear the timer interrupt. Necesario para lanzar la pr�xima interrupci�n.
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

// Se pudo crear una fuci�n para el env�o al DAC (ej. DAC_write)

// Se pudo usar una bandera: se activa ac� en el handler, y se verifica en el
// ciclo del main. Si la bandera est� en alto, se manda al DAC. Al terminar,
// se apagar�a la bandera. As�, el handler �nicamente reiniciar�a el tempo-
// rizador y encender�a la bandera.
// Es importante que lo que se haga en el handler no tome m�s tiempo que el
// per�odo del temporizador.


    // El DAC espera 16 bits: 4 de configuraciones y 12 del dato
    pui32DataTx[0] = (uint32_t)((0b0111 << 12) | (0x0FFF & valor_ADC));

    // Send data
    for(ui32Index = 0; ui32Index < NUM_SPI_DATA; ui32Index++)
    {
        SSIDataPut(SSI0_BASE, pui32DataTx[ui32Index]);
    }

    // Wait until SSI0 is done transferring all the data in the transmit FIFO.
    while(SSIBusy(SSI0_BASE))
    {
    }

}

void
Timer1IntHandler(void)
{
    // This array is used for storing the data read from the ADC FIFO. It
    // must be as large as the FIFO for the sequencer in use.  This example
    // uses sequence 3 which has a FIFO depth of 1.  If another sequence
    // was used with a deeper FIFO, then the array size must be changed.
    uint32_t pui32ADC0Value[1];

    // Clear the timer interrupt. Necesario para lanzar la pr�xima interrupci�n.
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // Trigger the ADC conversion.
    ADCProcessorTrigger(ADC0_BASE, 3);

    // Wait for conversion to be completed.
    while(!ADCIntStatus(ADC0_BASE, 3, false))
    {
    }
    // Clear the ADC interrupt flag.
    ADCIntClear(ADC0_BASE, 3);

    // Read ADC Value.
    ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);
    valor_ADC = pui32ADC0Value[0];

}



//*****************************************************************************
// Configure SSI0 in master Freescale (SPI) mode.
// Se configura el timer y su interrupci�n. El SPI se configura para enviar 16
// bits. El env�o se hace en la rutina de interrupci�n del timer.
//*****************************************************************************
int
main(void)
{
    uint32_t pui32residual[NUM_SPI_DATA];
    uint16_t freq_timer = 1000;    // En Hz
    uint16_t freq_muestreo = 250;    // En Hz

// ------ Configuraci�n del reloj ---------------------------------------------
    // Set the clock to run at 80 MHz (200 MHz / 2.5) using the PLL.
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ); // 80 MHz. S� FUNCIONA
// ----------------------------------------------------------------------------
    // The ADC0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);    // information on the ADC sequences and steps, reference the datasheet.
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);

    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    ADCIntClear(ADC0_BASE, 3);

    // Enable the peripherals used by this example.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    // ********************************ADC

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

        // El tercer argumento determina la frecuencia. El reloj se puede obtener
        // con SysCtlClockGet().
        // La frecuencia est� dada por SysCtlClockGet()/(valor del 3er argumento).
        // Ejemplos: Si se pone SysCtlClockGet(), la frecuencia ser� de 1 Hz.
        //           Si se pone SysCtlClockGet()/1000, la frecuencia ser� de 1 kHz
        //TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
        TimerLoadSet(TIMER0_BASE, TIMER_A, (uint32_t)(SysCtlClockGet()/freq_muestreo));

        // Setup the interrupt for the timer timeout.
        IntEnable(INT_TIMER0A);
        TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

        // Enable the timers.
        TimerEnable(TIMER0_BASE, TIMER_A);
//***************************************************

    // For this example SSI0 is used with PortA[5:2].  The actual port and pins
    // used may be different on your part, consult the data sheet for more
    // information.  GPIO port A needs to be enabled so these pins can be used.
    // TODO: change this to whichever GPIO port you are using.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    // Configure the GPIO settings for the SSI pins.  This function also gives
    // control of these pins to the SSI hardware.  Consult the data sheet to
    // see which functions are allocated per pin.
    // The pins are assigned as follows:
    //      PA5 - SSI0Tx
    //      PA4 - SSI0Rx
    //      PA3 - SSI0Fss
    //      PA2 - SSI0CLK
    // TODO: change this to select the port/pin you are using.
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 |
                   GPIO_PIN_2);

    // Configure and enable the SSI port for SPI master mode.  Use SSI0,
    // system clock supply, idle clock level low and active low clock in
    // freescale SPI mode, master mode, 4MHz SSI frequency, and 16-bit data.
    // For SPI mode, you can set the polarity of the SSI clock when the SSI
    // unit is idle.  You can also configure what clock edge you want to
    // capture data on.  Please reference the datasheet for more information on
    // the different SPI modes.
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, SPI_FREC, SPI_ANCHO);

    // Enable the SSI0 module.
    SSIEnable(SSI0_BASE);

    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the SPI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    while(SSIDataGetNonBlocking(SSI0_BASE, &pui32residual[0]))
    {
    }

    // Enable the peripherals used by this example.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Enable processor interrupts.
    IntMasterEnable();

    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // El tercer argumento determina la frecuencia. El reloj se puede obtener
    // con SysCtlClockGet().
    // La frecuencia est� dada por SysCtlClockGet()/(valor del 3er argumento).
    // Ejemplos: Si se pone SysCtlClockGet(), la frecuencia ser� de 1 Hz.
    //           Si se pone SysCtlClockGet()/1000, la frecuencia ser� de 1 kHz
    TimerLoadSet(TIMER0_BASE, TIMER_A, (uint32_t)(SysCtlClockGet()/freq_timer));

    // Setup the interrupt for the timer timeout.
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Enable the timers.
    TimerEnable(TIMER0_BASE, TIMER_A);

    // El env�o se hace al darse la interrupci�n del timer, para que sea en
    // intervalos regulares. Luego de las configuraciones, el programa se
    // queda en un ciclo infinito haciendo nada, esperando la interrupci�n.
    while(1)
    {
    }
}
