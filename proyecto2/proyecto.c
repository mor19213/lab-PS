/*
 * ADC y DAC y UART
 * proyecto (2)
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/rom.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/tm4c123gh6pm.h"


//*****************************************************************************
#define NUM_SPI_DATA    1  // N�mero de palabras que se env�an cada vez
#define VALOR_MAX    4095  // Valor m�ximo del contador
#define SPI_FREC  4000000  // Frecuencia para el reloj del SPI
#define SPI_ANCHO      16  // N�mero de bits que se env�an cada vez, entre 4 y 16

float y_n = 0;
float x_n = 0;

int i = 0;
uint16_t cont = 0;
uint16_t salida = 0;
uint32_t recibido = 0;

int modo = 0; // 0 - live, 1 - on-D
int proceso = 0;
int num_muestras_ond = 5000; // muestras a enviar en on-demand
char* StrNumMuestras[];
char* StrN0[], StrN1[], StrN2[], StrN3[], StrN4[], StrD0[], StrD1[], StrD2[], StrD3[], StrD4[];
float n0=0, n1=0, n2=0, n3=0, n4=0, d0=0, d1=0, d2=0, d3=0, d4=0;


void InitUART(void);
void UARTInt(int num);
void UARTSend(const char *pui8Buffer, uint32_t ui32Count);
void UARTSend(const char *pui8Buffer, uint32_t ui32Count){
    // Loop while there are more characters to send.
    while(ui32Count--){
        // Write the next character to the UART.
        //UARTCharPutNonBlocking(UART0_BASE, *pui8Buffer++);
        UARTCharPut(UART0_BASE, *pui8Buffer++);
    }
}

void UARTInt(int num){

    int enviar = 0, ceros = 0, contar =1;
    if (num == 0){
        UARTCharPut(UART0_BASE, (char)(48));
    } else {
        char str;
        int mod = 0;
        while (num != 0) {
            mod = num % 10;
            enviar = enviar * 10 + mod;
            num /= 10;
            if (mod == 0 && contar==1){
                ceros++;
            } else if(mod !=0){
                contar = 0;
            }
          }
        while(enviar > 0){
            mod = enviar % 10;
            str = (char)(mod + 48);
            UARTCharPut(UART0_BASE, str);
            SysCtlDelay((SysCtlClockGet()/3/1000));
            enviar /= 10;
        }
        while (ceros > 0){
            str = (char)(48);
            UARTCharPut(UART0_BASE, str);
            SysCtlDelay((SysCtlClockGet()/3/1000));
            ceros--;
        }
    }
}

void InitUART(void){
    /*Enable the GPIO Port A*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /*Enable the peripheral UART Module 0*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);


    /* Make the UART pins be peripheral controlled. */
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Sets the configuration of a UART. */
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_TX);

    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}


//*****************************************************************************
// INTERRUPCIONES
//*****************************************************************************

void UARTIntHandler(void){
    uint32_t ui32Status;

    // Get the interrrupt status.
    //
    //ui32Status = ROM_UARTIntStatus(UART0_BASE, true);
    ui32Status = UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    //ROM_UARTIntClear(UART0_BASE, ui32Status);
    UARTIntClear(UART0_BASE, ui32Status);
    // Loop while there are characters in the receive FIFO.
    //
    char ch;
    while(UARTCharsAvail(UART0_BASE))
    {
        ch = UARTCharGetNonBlocking(UART0_BASE);
        if (proceso == 0){
            switch (ch){
            case 68: // "D" on-demand, recibir la cantidad de muestras a enviar
                modo = 1;
                proceso = 1;
                *StrNumMuestras = strcpy(*StrNumMuestras, "");
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // azul
                break;
            case 76: // "L" Live, no se recibe nada mas
                modo = 2;
                break;
            case 84: // "T" Tiva, recibir los coeficientes
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // rojo
                proceso = 1;
                modo = 3;
                *StrD0 = strcpy(*StrD0, "");
                *StrD1 = strcpy(*StrD1, "");
                *StrD2 = strcpy(*StrD2, "");
                *StrD3 = strcpy(*StrD3, "");
                *StrD4 = strcpy(*StrD4, "");
                *StrN0 = strcpy(*StrN0, "");
                *StrN1 = strcpy(*StrN1, "");
                *StrN2 = strcpy(*StrN2, "");
                *StrN3 = strcpy(*StrN3, "");
                *StrN4 = strcpy(*StrN4, "");
                break;
            case 82: // "R" Record,
                proceso = 1;
                modo = 6;
                break;
            }
        }

        if (proceso == 1 && modo == 1){
            if (ch == 44){
                proceso = 0;
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // azul
            } else {
                *StrNumMuestras = strcat(*StrNumMuestras, ch);
            }
        } else if (modo == 3){
            //UARTSend("tiva", 4);
            switch (proceso){
                    case 1:
                        if (ch == 44){
                            d0 = atof(StrD0);
                            proceso++;
                            //UARTSend("coef", 4);
                        } else {
                            *StrD0 = strcat(*StrD0, ch);
                        }
                        break;
                    case 2:
                        if (ch == 44){
                            d1 = atof(StrD1);
                            proceso++;
                            //UARTSend("coef", 4);
                        } else {
                            *StrD1 = strcat(*StrD1, ch);
                        }
                        break;
                    case 3:
                        if (ch == 44){
                            d2 = atof(StrD2);
                            proceso++;
                            //UARTSend("coef", 4);
                        } else {
                            *StrD2 = strcat(*StrD2, ch);
                        }
                        break;
                    case 4:
                        if (ch == 44){
                            d3 = atof(StrD3);
                            proceso++;
                            //UARTSend("coef", 4);
                        } else {
                            *StrD3 = strcat(*StrD3, ch);
                        }
                        break;
                    case 5:
                        if (ch == 44){
                            d4 = atof(StrD4);
                            proceso++;
                            //UARTSend("coef", 4);
                        } else {
                            *StrD4 = strcat(*StrD4, ch);
                        }
                        break;
                    case 6:
                        if (ch == 44){
                            n0 = atof(StrN0);
                            proceso++;
                            //UARTSend("coef", 4);
                        } else {
                            *StrN0 = strcat(*StrN0, ch);
                        }
                        break;
                    case 7:
                        if (ch == 44){
                            n1 = atof(StrN1);
                            proceso++;
                            //UARTSend("coef", 4);
                        } else {
                            *StrN1 = strcat(*StrN1, ch);
                        }
                        break;
                    case 8:
                        if (ch == 44){
                            n2 = atof(StrN2);
                            proceso++;
                            //UARTSend("coef", 4);
                        } else {
                            *StrN2 = strcat(*StrN2, ch);
                        }
                        break;
                    case 9:
                        if (ch == 44){
                            n3 = atof(StrN3);
                            proceso++;
                            //UARTSend("coef", 4);
                        } else {
                            *StrN3 = strcat(*StrN3, ch);
                        }
                        break;
                    case 10:
                        if (ch == 44){
                            n4 = atof(StrN4);
                            proceso=0;
                            //UARTSend("coef", 4);
                            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // rojo
                        } else {
                            *StrN4 = strcat(*StrN4, ch);
                        }
                        break;

                    }
        }
        num_muestras_ond = atof(StrNumMuestras);
        if (proceso == 2)
    }
    float recibido = atof(StrD1);
    /*
    if (strncmp("bruno", ch, 5)){
        UARTSend("Burrito", 7);
    } else {
        UARTSend("no bruno :(", 7);

    }
*/
    if ( recibido > 0 ){
        UARTSend("MAyor", 5);
    } else if (recibido < 0){
        UARTSend("Menor", 5);
    }
}



void
Timer0IntHandler(void) //DAC
{
    uint32_t pui32DataTx[NUM_SPI_DATA]; // la funci�n put pide tipo uint32_t
    uint8_t ui32Index;

    // Clear the timer interrupt. Necesario para lanzar la pr�xima interrupci�n.
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    salida = y_n;   // ENVIAR DATO SALIDO DEL FILTRO
    pui32DataTx[0] = (uint32_t)((0b0111 << 12) | (0x0FFF & salida));

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
    uint32_t pui32ADC0Value[1];

    // Clear the timer interrupt. Necesario para lanzar la pr�xima interrupci�n.
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    ADCProcessorTrigger(ADC0_BASE, 3);
    while(!ADCIntStatus(ADC0_BASE, 3, false))  // Wait for conversion to be completed.
    {
    }
    // Clear the ADC interrupt flag.
    ADCIntClear(ADC0_BASE, 3);

    // Read ADC Value.
    ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);
    y_n = pui32ADC0Value[0];
    x_n = pui32ADC0Value[0];

    if (modo == 0){ // ENVIAR
    } else{
    }
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
    uint16_t freq_timer0 = 1000;    // En Hz
    uint16_t freq_muestreo = 1000;    // En Hz

// ------ Configuraci�n del reloj ---------------------------------------------
    // Set the clock to run at 80 MHz (200 MHz / 2.5) using the PLL.
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ); // 80 MHz. S� FUNCIONA
// ---------------
    // The ADC0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // For this example ADC0 is used with AIN0 on port E3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);
    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    ADCIntClear(ADC0_BASE, 3);

    // Enable the peripherals used by this example.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    // Enable processor interrupts.
    IntMasterEnable();

    // ------------------- TIMER 1 -------------------------------------------------------
    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

    // El tercer argumento determina la frecuencia. El reloj se puede obtener
    // con SysCtlClockGet().
    // La frecuencia est� dada por SysCtlClockGet()/(valor del 3er argumento).
    //TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
    TimerLoadSet(TIMER1_BASE, TIMER_A, (uint32_t)(SysCtlClockGet()/freq_muestreo));
    // Setup the interrupt for the timer timeout.
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    TimerEnable(TIMER1_BASE, TIMER_A);
//*******************************************************************************
    // Las conversiones se hacen al darse la interrupci�n del timer, para que
    // el muestreo sea preciso. Luego de las configuraciones, el programa se
    // queda en un ciclo infinito haciendo nada.

    // salida Señal Cuadrada
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);

    // The SSI0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 |
                   GPIO_PIN_2);
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, SPI_FREC, SPI_ANCHO);

    SSIEnable(SSI0_BASE);
    while(SSIDataGetNonBlocking(SSI0_BASE, &pui32residual[0]))
    {
    }

    // ----------------- TIMER 0 --------------------------------------------
    // Enable the peripherals used by this example.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Enable processor interrupts.
    IntMasterEnable();

    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // El tercer argumento determina la frecuencia. El reloj se puede obtener
    // con SysCtlClockGet().
    // La frecuencia est� dada por SysCtlClockGet()/(valor del 3er argumento).
    TimerLoadSet(TIMER0_BASE, TIMER_A, (uint32_t)(SysCtlClockGet()/freq_timer0));

    // Setup the interrupt for the timer timeout.
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Enable the timers.
    TimerEnable(TIMER0_BASE, TIMER_A);

    // UART
    volatile uint32_t ui32Loop;
    IntEnable(INT_UART0);
    InitUART();

    int num = 0;

    // ------------------------ CONFIG LEDS ----------------------------------
    // Enable the GPIO port that is used for the on-board LED.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Check if the peripheral access is enabled.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    //
    // Enable the GPIO pin for the LED (PG2).  Set the direction as output, and
    // enable the GPIO pin for digital function.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);  // azul
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);  // rojo
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);  // verde
    // -----------------------------------------------------------------------
    while(1){
        UARTInt(num);
        SysCtlDelay(400*(SysCtlClockGet()/3/1000));
        UARTCharPut(UART0_BASE, (char)(10));
        num += 3;
    }
}
