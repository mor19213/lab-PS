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

    int enviar = 0, ceros = 0, contar =1;
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
        SysCtlDelay(100*(SysCtlClockGet()/3/1000));
        enviar /= 10;
    }
    while (ceros > 0){
        str = (char)(48);
        UARTCharPut(UART0_BASE, str);
        SysCtlDelay(100*(SysCtlClockGet()/3/1000));
        ceros--;

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
    /*
    char ch[100];
    int i = 0;
    while(UARTCharsAvail(UART0_BASE))
    {
        //
        // Read the next character from the UART and write it back to the UART.
        ch[i] = UARTCharGetNonBlocking(UART0_BASE);
        i++;
    }*/

}

void Timer0IntHandler(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
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

           IntMasterEnable();
           IntEnable(INT_UART0);
           InitUART();
/*
       SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
       // CONFIG del timer0 como temporizador periodico
       TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
       ui32Period = (SysCtlClockGet())*2;   // periodo para el temporizador (2 segundos)
       TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -  1); // establece el periodo para el timer
       IntEnable(INT_TIMER0A);
       TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
       IntMasterEnable();
       TimerEnable(TIMER0_BASE, TIMER_A);
       */
/*
       int num = 820100, enviar = 0, ceros = 0, contar =1;
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
           SysCtlDelay(100*(SysCtlClockGet()/3/1000));
           enviar /= 10;
       }
       while (ceros > 0){
           str = (char)(48);
           UARTCharPut(UART0_BASE, str);
           SysCtlDelay(100*(SysCtlClockGet()/3/1000));
           UARTCharPut(UART0_BASE, str);
           SysCtlDelay(100*(SysCtlClockGet()/3/1000));
           ceros--;

       }*/
       //UARTInt(num);
       int num = 89;
       while(1){
           UARTInt(num);
           SysCtlDelay(2000*(SysCtlClockGet()/3/1000));
           UARTCharPut(UART0_BASE, (char)(10));
           num += 3;
//           num = rand(
       }

}


