#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#include <string.h>

 void uartIntHandler(void);
uint32_t ui32Period = 0;
uint32_t recibido = 0;
uint8_t rojo = 0;
uint8_t verde = 0;
uint8_t azul = 0;

/**
 * main.c
 */
void InitUART(void);
void uart_rx(void);

void UARTSend(const char *pui8Buffer, uint32_t ui32Count);
void IntToString(int num,  char *str);
void UARTInt(int num);


void IntToString(int num, char *str){
            *str = (char*)(num + 48);
}


void UARTInt(int num){
    // Loop while there are more characters to send.
    char* str;
    int mod = 0;
    while(num > 0){
        mod = num % 10;
        //*str = (char*)(mod + 48);
        IntToString(mod, str);
        UARTCharPut(UART0_BASE, *str);
        SysCtlDelay(100*(SysCtlClockGet()/3/1000));
        num /= 10;
    }
}

void UARTSend(const char *pui8Buffer, uint32_t ui32Count){
    // Loop while there are more characters to send.
    while(ui32Count--){
        // Write the next character to the UART.
        //UARTCharPutNonBlocking(UART0_BASE, *pui8Buffer++);
        UARTCharPut(UART0_BASE, *pui8Buffer++);
    }
}


void UARTIntHandler(void){
    uart_rx();
    //UARTCharPut(UART0_BASE, 'a');
}

void Timer0IntHandler(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2)){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    } else {
        if (azul == 1){
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        }
    }
    if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1)){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
    } else {
        if (rojo == 1){
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        }
    }
    if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3)){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    } else {
        if (verde == 1){
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
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

void uart_rx(void){
    recibido = UARTCharGet(UART0_BASE);
        UARTCharPut(UART0_BASE, recibido);
    if (recibido == 'r'){
        UARTSend("rojo", 4);
    }
    if (recibido == 'g'){
        UARTSend("verde", 5);
    }
    if (recibido == 'b'){
        UARTSend("azul", 4);
    }
}



int main(void)
{
    volatile uint32_t ui32Loop;

    SysCtlClockSet(
            SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ
                    | SYSCTL_OSC_MAIN);
    //
           // Enable the GPIO port that is used for the on-board LED.
           //
           SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

           //
           // Check if the peripheral access is enabled.
           //
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

           IntMasterEnable();
           IntEnable(INT_UART0);


           InitUART();

       SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
       // CONFIG del timer0 como temporizador periodico
       TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
       ui32Period = (SysCtlClockGet())*2;   // periodo para el temporizador (2 segundos)
       TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -  1); // establece el periodo para el timer
       IntEnable(INT_TIMER0A);
       TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
       IntMasterEnable();
       TimerEnable(TIMER0_BASE, TIMER_A);
       char* str[50];
       //sprintf(&str, "%d", 42);
       int num = 5;
       //IntToString(num, 1);
       *str = (char*)(num + 48);
       //IntToString(2, 1, str1);
       //IntToString(3, 1, str2);
       //*str = strcat(str1, str2);
       UARTSend(*str, 1);
       //UARTInt(num);
       while(1){
           }

}


