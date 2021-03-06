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

float yn = 0, yn_1=0, yn_2=0, yn_3=0, yn_4=0, yn_5 = 0, yn_6=0, yn_7=0, yn_8=0;
float xn = 0, xn_1=0, xn_2=0, xn_3=0, xn_4=0, xn_5 = 0, xn_6=0, xn_7=0, xn_8=0;

int i = 0;
uint16_t cont = 0;
uint16_t salida = 0;
uint32_t recibido = 0;

int modo = 0, live = 0, demand = 0; // 0 - live, 1 - on-D
int proceso = 0, num = 0, den=0;
int grabacion[1000];
float NumMuestrasD = 5000, NumMuestrasR = 0;
int MuestraR=1, grabar=0;
char StrNumMuestras[10];
float temp = 0;
int DigNumMues = 0, DigCoef = 0;
int banderita = 0;
char StrN0[5], StrN1[5], StrN2[5], StrN3[5], StrN4[5], StrN5[5], StrN6[5], StrN7[5], StrN8[5], StrD0[5], StrD1[5], StrD2[5], StrD3[5], StrD4[5], StrD5[5], StrD6[5], StrD7[5], StrD8[5];
float b0=1, b1=0, b2=0, b3=0, b4=0, b5=0, b6=0, b7=0, b8=0, a0=0, a1=0, a2=0, a3=0, a4=0, a5=0, a6=0, a7=0, a8=0;


void InitUART(void);
void UARTInt(int num);
void UARTSend(const char *pui8Buffer, uint32_t ui32Count);
void UARTSend(const char *pui8Buffer, uint32_t ui32Count){
    while(ui32Count--){
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
            SysCtlDelay((SysCtlClockGet()/100000));
            enviar /= 10;
        }
        while (ceros > 0){
            str = (char)(48);
            UARTCharPut(UART0_BASE, str);
            SysCtlDelay((SysCtlClockGet()/100000));
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
    ui32Status = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, ui32Status);
    char ch;
    while(UARTCharsAvail(UART0_BASE))
    {
        ch = UARTCharGetNonBlocking(UART0_BASE);

        if (proceso == 1 && modo == 1){
            if (ch == 10){
                proceso = 0;
                NumMuestrasD = atof(StrNumMuestras);
                temp = NumMuestrasD;
                DigNumMues = 0;
                demand = 1;
            } else {
                StrNumMuestras[DigNumMues] = ch;
                DigNumMues++;
            }
        }
        if (modo == 3){
            switch (proceso){
                    case 1:
                        if (ch == 44){
                            a0 = atof(StrD0);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrD0[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 2:
                        if (ch == 44){
                            a1 = atof(StrD1);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrD1[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 3:
                        if (ch == 44){
                            a2 = atof(StrD2);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrD2[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 4:
                        if (ch == 44){
                            a3 = atof(StrD3);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrD3[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 5:
                        if (ch == 44){
                            a4 = atof(StrD4);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrD4[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 6:
                        if (ch == 44){
                            a5 = atof(StrD5);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrD5[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 7:
                        if (ch == 44){
                            a6 = atof(StrD6);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrD6[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 8:
                        if (ch == 44){
                            a7 = atof(StrD7);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrD7[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 9:
                        if (ch == 44){
                            a8 = atof(StrD8);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrD8[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 10:
                        if (ch == 44){
                            b0 = atof(StrN0);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrN0[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 11:
                        if (ch == 44){
                            b1 = atof(StrN1);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrN1[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 12:
                        if (ch == 44){
                            b2 = atof(StrN2);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrN2[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 13:
                        if (ch == 44){
                            b3 = atof(StrN3);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrN3[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 14:
                        if (ch == 10){
                            b4 = atof(StrN4);
                            proceso=0;
                            DigCoef = 0;
                        } else {
                            StrN4[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;

                    case 15:
                        if (ch == 44){
                            b5 = atof(StrN5);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrN5[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 16:
                        if (ch == 44){
                            b6 = atof(StrN6);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrN6[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 17:
                        if (ch == 44){
                            b7 = atof(StrN7);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrN7[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    case 18:
                        if (ch == 44){
                            b8 = atof(StrN8);
                            proceso++;
                            DigCoef = 0;
                        } else {
                            StrN8[DigCoef] = ch;
                            DigCoef++;
                        }
                        break;
                    }
        } else if (proceso == 1 && modo == 4){
            if (ch == 10){
                proceso = 0;
                NumMuestrasR = atof(StrNumMuestras);
                DigNumMues = 0;
                MuestraR = 0;
                grabar = 1;
            } else {
                StrNumMuestras[DigNumMues] = ch;
                DigNumMues++;
            }
        }
        if (proceso == 0){
            switch (ch){
            case 68: // "D" on-demand, recibir la cantidad de muestras a enviar
                NumMuestrasD = 0;
                modo = 1;
                proceso = 1;
                strcpy(StrNumMuestras, "");
                break;
            case 76: // "L" Live, no se recibe nada mas
                modo = 2;
                if (live ==1){
                    live = 0;
                }else{
                    live = 1;
                }
                break;
            case 84: // "T" Tiva, recibir los coeficientes
               // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // ROJO
                proceso = 1;
                modo = 3;
                strcpy(StrD0, "");
                strcpy(StrD1, "");
                strcpy(StrD2, "");
                strcpy(StrD3, "");
                strcpy(StrD4, "");
                strcpy(StrN0, "");
                strcpy(StrN1, "");
                strcpy(StrN2, "");
                strcpy(StrN3, "");
                strcpy(StrN4, "");
                break;
            case 82: // "R" Record, recibir la cantidad de muestras
                proceso = 1;
                modo = 4;
                break;
            }
        }
    }
}


void
Timer0IntHandler(void){
    uint32_t pui32DataTx[NUM_SPI_DATA]; // la funci�n put pide tipo uint32_t
    uint8_t ui32Index;

    // Clear the timer interrupt. Necesario para lanzar la pr�xima interrupci�n.
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    salida = yn;   // ENVIAR DATO SALIDO DEL FILTRO
    pui32DataTx[0] = (uint32_t)((0b0111 << 12) | (0x0FFF & salida));

    // Send data
    for(ui32Index = 0; ui32Index < NUM_SPI_DATA; ui32Index++){
        SSIDataPut(SSI0_BASE, pui32DataTx[ui32Index]);
    }
    // Wait until SSI0 is done transferring all the data in the transmit FIFO.
    while(SSIBusy(SSI0_BASE)){
    }

    uint32_t pui32ADC0Value[1];

    ADCProcessorTrigger(ADC0_BASE, 3);
    while(!ADCIntStatus(ADC0_BASE, 3, false)){
    }
    // Clear the ADC interrupt flag.
    ADCIntClear(ADC0_BASE, 3);

    // Read ADC Value.
    ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);
    // Filtro
    xn_1=xn;
    xn_2=xn_1;
    xn_3=xn_2;
    xn_4=xn_3;
    xn_5=xn_4;
    xn_6=xn_5;
    xn_7=xn_6;
    xn_8=xn_7;
    xn = pui32ADC0Value[0];
    yn_1=yn;
    yn_2=yn_1;
    yn_3=yn_2;
    yn_4=yn_3;
    yn_5=yn_4;
    yn_6=yn_5;
    yn_7=yn_6;
    yn_8=yn_7;
    num = b0*xn +b1*xn_1 +b2*xn_2 +b3*xn_3 +b4*xn_4 +b5*xn_5 +b6*xn_6 +b7*xn_7 +b8*xn_8;
    den = -a1*yn_1 -a2*yn_2 -a3*yn_3 -a4*yn_4-a5*yn_5 -a6*yn_6 -a7*yn_7 -a8*yn_8;
    yn =  num+den;
    if (yn < 0){
        yn = 0;
    } else if (yn > 4095){
        yn = 4095;
    }
    if (demand == 1){
        if (NumMuestrasD > 0){
            UARTInt(xn);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // AZUL
            UARTCharPut(UART0_BASE, (char)(10));
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // ROJO
        //banderita = 1;
        NumMuestrasD--;
        }else{
            demand = 0;
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // ROJO
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); //rojo
        }
    }
    else if (live == 1){ // ENVIAR LIVE
        banderita = 1;
        }
    if (grabar == 1){
        if (NumMuestrasR >= MuestraR){
            grabacion[MuestraR] = xn;
            MuestraR++;
        }
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
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    // Enable processor interrupts.
    IntMasterEnable();
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
        /*
        UARTInt(num);
        SysCtlDelay(400*(SysCtlClockGet()/3/1000));
        UARTCharPut(UART0_BASE, (char)(10));
        num += 3;
        */

    if (banderita == 1){
        UARTInt(xn);
        UARTCharPut(UART0_BASE, (char)(10));
        SysCtlDelay((SysCtlClockGet()/1000));
    }
    }
}
