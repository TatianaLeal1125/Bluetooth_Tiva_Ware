/*
 * main.c
 */
//#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"											//********
#include "stdbool.h"										//********
#include "inc/hw_types.h"  //tipos de hardware				//********
#include "inc/hw_memmap.h" //mapeo de memoria, y registros	//********
#include "inc/hw_sysctl.h" //habilitar peripherals TIVA     //********
#include "inc/hw_gpio.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "inc/hw_timer.h"
#include "inc/hw_uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"  //appi para el systemctrl	//********
#include "driverlib/gpio.h" //appi del gpio
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.c"

#define rojo 0x02
#define verde 0x08
#define azul 0x04
#define sw1 0x01   //PF0
#define sw2 0x10   //PF4
#define todos 0x0E // = 0x02|0x04|0x08
short bandera=0;
int aux[3];
int i=0;
//**Preparar el puerto SPI 0 en modo maestro
//-------------------------------------------------------------------------
void irs_uart1_rx(void)
{
	unsigned long status1;
	status1 = UARTIntStatus(UART1_BASE,true);				//estado de interrup requerida
	UARTIntClear(UART1_BASE,status1);
	bandera=1;
	//GPIO_PORTF_DATA_R^=azul;
	//SysCtlDelay(3666666);
	//GPIO_PORTF_DATA_R^=azul;
	//while
		//UARTprintf("\ninterrupcion\n");
		//UARTCharPutNonBlocking(UART0_BASE,dato);
}

void irs_uart0_rx(void){
	unsigned long status0;
	unsigned char dato;
	status0 = UARTIntStatus(UART0_BASE,true);				//estado de interrup requerida
	UARTIntClear(UART0_BASE,status0);
	GPIO_PORTF_DATA_R^=verde;
	SysCtlDelay(3666666);
	GPIO_PORTF_DATA_R^=verde;
	while(UARTCharsAvail(UART0_BASE))
		{
		dato = UARTCharGetNonBlocking(UART0_BASE);
		UARTCharPutNonBlocking(UART1_BASE,dato);
	    }
}//isr_uart0_rx

//**Preparar el gpio puerto F
//-------------------------------------------------------------------------
void iniciagpio(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);	//Habilitar GPIO
	/*GPIODirModeSet(GPIO_PORTF_BASE,   //dirección GPIO_PORTF_BASE
	verde|rojo|azul,
	GPIO_DIR_MODE_OUT); */         //configura como salida
	GPIOPinWrite(GPIO_PORTF_BASE,todos,0);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,todos);
	/*GPIO_PORTF_LOCK_R = 0x4C4F434B; // Habilita escritura en GPIOCR
	GPIO_PORTF_CR_R |=1;			//
	//GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,sw1|sw2);
	/*GPIOPadConfigSet(GPIO_PORTF_BASE,	//Habilita las resistencias de pull up en FP0 y FP4
			sw1|sw2,
			0,
			GPIO_PIN_TYPE_STD_WPU);*/
	//GPIOIntEnable(GPIO_PORTF_BASE,GPIO_INT_PIN_0|GPIO_INT_PIN_4);
	//GPIOIntTypeSet(GPIO_PORTF_BASE,sw1|sw2,GPIO_FALLING_EDGE);
	//IntEnable(INT_GPIOF_BLIZZARD);
}

//**preparar el modulo UART0
//------------------------------------------------------------
void iniciauart_0(void){
	//habilitando la función de periférico
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	//habilitamos la función de uart0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	//configuramos el puerto para la funcón deseada
	GPIOPinTypeUART(GPIO_PORTA_BASE,GPIO_PIN_0|GPIO_PIN_1);
	//configuramos los pines del puerto A, para la función de UART. la encontramos en pin_map
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 9600, 16000000);
	UARTIntEnable(UART0_BASE,UART_INT_RT | UART_INT_RI);     //configuramos las interrup de la uart
	UARTEnable(UART0_BASE);						  //habilitamos el módulo uart
	IntEnable(INT_UART0_BLIZZARD);                //habilita la interrupción
}

void iniciauart_1(void){
	//habilitando la función de periférico
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	//habilitamos la función de uart0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	//configuramos el puerto para la funcón deseada
	GPIOPinTypeUART(GPIO_PORTB_BASE,GPIO_PIN_0|GPIO_PIN_1);
	//configuramos los pines del puerto A, para la función de UART. la encontramos en pin_map
	GPIOPinConfigure(GPIO_PB0_U1RX);
	GPIOPinConfigure(GPIO_PB1_U1TX);
	UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);
    UARTConfigSetExpClk(UART1_BASE,16000000,9600,UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE);
	UARTIntEnable(UART1_BASE,UART_INT_RT | UART_INT_RX);     //configuramos las interrup de la uart
	UARTEnable(UART1_BASE);						  //habilitamos el módulo uart
	IntEnable(INT_UART1_BLIZZARD);                //habilita la interrupción
}
//------------------------------------------------------------
//**Servicio de interrupción de UART0 Recepcion
//------------------------------------------------------------
void iniciatimer0(void){
	long contador;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);	//Habilitar TIMER0
	TimerConfigure(TIMER0_BASE,TIMER_CFG_PERIODIC);
	//contador=16000000; // un segundo
	contador=100000;
	TimerLoadSet(TIMER0_BASE,TIMER_A,contador); 	//carga la base de tiempo del timerA
	TimerIntEnable(TIMER0_BASE,TIMER_TIMA_TIMEOUT);	//habilita la interrupción por match del timer0
	TimerEnable(TIMER0_BASE,TIMER_BOTH); 	//HABILITA TIMER 0 (SUBTIMERA Y SUBTIMERB)
	//IntEnable(INT_TIMER0A_BLIZZARD);
}

void InterruptTimer0(void){
	unsigned long status;
	status=TimerIntStatus(TIMER0_BASE, true);
	TimerIntClear(TIMER0_BASE, status);
	//GPIOPinWrite(GPIO_PORTF_BASE,azul,azul);
	//ssiban=1;
	i++;
	if(i%8==0)
		{GPIO_PORTF_DATA_R|=aux[0];
		GPIO_PORTF_DATA_R^=aux[1];}
	else
		{GPIO_PORTF_DATA_R|=aux[1];
		GPIO_PORTF_DATA_R^=aux[0];
		}
}

int main(void) {
	unsigned char dato;
	SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_USE_OSC | SYSCTL_XTAL_16MHZ); //cristal de 16 MHz
	iniciagpio();
	iniciauart_0();
	iniciauart_1();
	iniciatimer0();
	//UARTSend((uint8_t *)"\033[2JEnter text: ", 16);
	IntMasterEnable();
    while(1){
	while(bandera==0);
    while(UARTCharsAvail(UART1_BASE))
    		{
    		dato = UARTCharGetNonBlocking(UART1_BASE);
    		if (dato=='v'){
    			IntDisable(INT_TIMER0A_BLIZZARD);
    			GPIO_PORTF_DATA_R|=verde;}
    		else if (dato=='b'){
    			IntDisable(INT_TIMER0A_BLIZZARD);
    			GPIO_PORTF_DATA_R|=todos;}
    		else if (dato=='r'){
    			IntDisable(INT_TIMER0A_BLIZZARD);
    			GPIO_PORTF_DATA_R|=rojo;}
    		else if (dato=='a'){
    			IntDisable(INT_TIMER0A_BLIZZARD);
    			GPIO_PORTF_DATA_R|=azul;}
    		else if (dato=='p')
    			{aux[0]=rojo;
    			 aux[1]=azul;
    			 aux[2]=0;
    			//GPIO_PORTF_DATA_R|=azul;
    			//GPIO_PORTF_DATA_R|=verde;
    			IntEnable(INT_TIMER0A_BLIZZARD);}
    		else if (dato=='m')
    		    {aux[0]=verde;
    		     aux[1]=azul;
    		     aux[2]=0;
    		    //GPIO_PORTF_DATA_R|=azul;
    		    //GPIO_PORTF_DATA_R|=verde;
    		    IntEnable(INT_TIMER0A_BLIZZARD);}
    		else if (dato=='n'){
    			IntDisable(INT_TIMER0A_BLIZZARD);
    			GPIO_PORTF_DATA_R=0x00;}
    		UARTCharPutNonBlocking(UART0_BASE,dato);
    	    bandera=0;
    		}
}}
