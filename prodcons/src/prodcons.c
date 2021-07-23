#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include "driverbuttons.h" // Projects/drivers
#include "stdbool.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"

#define BUFFER_SIZE 8

void initLed();

osThreadId_t consumidor_id;
osSemaphoreId_t cheio_id, vazio_id;
int contador = 0;
int tickAnterior = 0;
uint8_t buffer[BUFFER_SIZE];

void GPIOJ_Handler(void){
  int tick = osKernelGetTickCount();
  
  if(tick - tickAnterior >= 172) {
    osSemaphoreAcquire(vazio_id, osWaitForever);
    buffer[0] = contador;
    if(contador >= 16) 
    {
      contador = 0;
    }
    contador++;
    osSemaphoreRelease(cheio_id); 
    tickAnterior = tick;
  }
  
  GPIOIntClear(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
}

void consumidor(void *arg){
  while(1){
    osSemaphoreAcquire(cheio_id, osWaitForever);
    
    int valorLed = buffer[0];
    LEDWrite(LED4 | LED3 | LED2 | LED1, valorLed);
    
    osSemaphoreRelease(vazio_id);
  }
  
}

void main(void){
  SystemInit();
  
  initLed();
  
  for(int i = 0; i < 1000000; i++);
  
  osKernelInitialize();
  consumidor_id = osThreadNew(consumidor, NULL, NULL);
  
  vazio_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL);
  cheio_id = osSemaphoreNew(BUFFER_SIZE, 0, NULL);
  
  
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1){
  }
} // main

void initLed()
{
  LEDInit(LED4 | LED3 | LED2 | LED1);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); 
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
  GPIOPadConfigSet(GPIO_PORTJ_BASE ,GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
  GPIOIntDisable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
  GPIOIntTypeSet(GPIO_PORTJ_BASE,GPIO_PIN_0,GPIO_FALLING_EDGE);
  GPIOIntRegister(GPIO_PORTJ_BASE, GPIOJ_Handler);
  GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
}