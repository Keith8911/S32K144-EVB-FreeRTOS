/* I have created two task Task1 & Task2, and another function is an interrupt task.
 * Task 2 is always in suspend mode, so whenever you press BTN1 or BTN0 it will resume
 * Task2 and another LED will also toggle whenever you will press the BTN*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include<stdio.h>

#include "interrupt_manager.h"
#include "clock_manager.h"
#include "clockMan1.h"
#include "pin_mux.h"
#include "semphr.h"
#include "event_groups.h"
#include "BoardDefines.h"

#define LED_PORT 	PORTD
#define GPIO_PORT	PTD
#define PCC_CLOCK	PCC_PORTD_CLOCK
#define LED1		15U
#define LED2		16U
#define LED3		0U

#define BTN1			13U
#define BTN_GPIO        PTC
#define BTN1_PIN        13U
#define BTN2_PIN        12U
#define BTN_PORT        PORTC
#define BTN_PORT_IRQn   PORTC_IRQn

#define TASK2_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define	TASK1_PRIORITY		( tskIDLE_PRIORITY + 1 )


#define mainDONT_BLOCK						( 0UL )


static void prvSetupHardware( void );

void Task2( void *pvParameters );
void Task1( void *pvParameters );

TaskHandle_t xHandle1;
TaskHandle_t xHandle2;

/*-----------------------------------------------------------*/
void rtos_start( void )
{
	prvSetupHardware();

	xTaskCreate( Task1, "Task1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, &xHandle1 );
	xTaskCreate( Task2, "Task2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &xHandle2 );
	vTaskStartScheduler();

}

/*-----------------------------------------------------------*/

void Task1( void *pvParameters )
{
	(void)pvParameters;

	for( ;; )
	{
		PINS_DRV_TogglePins(LED_GPIO,(1<<LED1));
		vTaskDelay(1000);
	}
}

/*-----------------------------------------------------------*/

void Task2( void *pvParameters )
{
	(void)pvParameters;
	for( ;; )
	{
		vTaskSuspend(NULL);
		PINS_DRV_TogglePins(GPIO_PORT,(1 << LED2));
	}
}



/*-----------------------------------------------------------*/

void INT_Task2( void *pvParameters )
{
	PORTC->ISFR|=((1<<12)|(1<<13));
	BaseType_t CheckIfYieldRequired;
	CheckIfYieldRequired=xTaskResumeFromISR(xHandle2);
	portYIELD_FROM_ISR(CheckIfYieldRequired);
}

/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
                   g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    boardSetup();

    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
	PINS_DRV_SetPins(LED_GPIO, (1 << LED1) | (1 << LED2)|(1 << LED3));
	PINS_DRV_SetPinIntSel(BTN_PORT, BTN1_PIN, PORT_INT_RISING_EDGE);
	PINS_DRV_SetPinIntSel(BTN_PORT, BTN2_PIN, PORT_INT_RISING_EDGE);
	/* Install buttons ISR */
	INT_SYS_InstallHandler(BTN_PORT_IRQn, &INT_Task2, NULL);
	INT_SYS_SetPriority(BTN_PORT_IRQn, 3);
	/* Enable buttons interrupt */
	INT_SYS_EnableIRQ(BTN_PORT_IRQn);
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    volatile size_t xFreeHeapSpace;

	xFreeHeapSpace = xPortGetFreeHeapSize();

	if( xFreeHeapSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}

}
/*-----------------------------------------------------------*/

/* The Blinky build configuration does not include run time stats gathering,
however, the Full and Blinky build configurations share a FreeRTOSConfig.h
file.  Therefore, dummy run time stats functions need to be defined to keep the
linker happy. */
void vMainConfigureTimerForRunTimeStats( void ) {}
unsigned long ulMainGetRunTimeCounterValue( void ) { return 0UL; }

/* A tick hook is used by the "Full" build configuration.  The Full and blinky
build configurations share a FreeRTOSConfig.h header file, so this simple build
configuration also has to define a tick hook - even though it does not actually
use it for anything. */
void vApplicationTickHook( void ) {}

/*-----------------------------------------------------------*/


