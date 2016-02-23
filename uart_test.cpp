/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

#include <cstring>
#include <cstdio>


#include "ModbusMaster.h"
#include "LiquidCrystal.h"
#include "lcd_port.h"
#include "DecimalEdit.h"
#include "MenuItem.h"
#include "PropertyEdit.h"
#include "SimpleMenu.h"
#include "DebouncedInput.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/


/*****************************************************************************
 * Public functions
 ****************************************************************************/
static volatile int counter;
static volatile uint32_t systicks;
static volatile int systicks2;
static int dPort[4] = {0,0,1,0};
static int dPin[4] = {10,16,3,0};
static volatile bool tickFlag = false;

static volatile int ctr = 0;


// TODO: insert other definitions and declarations here

#define TICKRATE_HZ (1000)	/* 100 ticks per second */

#ifdef __cplusplus
extern "C" {
#endif

void RIT_IRQHandler(void)
{
	// Acknowledge interrupt by clearing RIT interrupt flag
	Chip_RIT_ClearIntStatus(LPC_RITIMER);
	// disable RIT – we want a single interrupt after the wait is over
	Chip_RIT_Disable(LPC_RITIMER);
	// set a flag to notify main program
	Flag = true;


}
void SysTick_Handler(void)
{

	systicks2++;
	Flag=true;
	if(counter > 0) {
		counter-=1;
	}
}
#ifdef __cplusplus
}
#endif

unsigned int GetSysticks()
{
	return systicks2;
}


void Sleep(int ms)
{
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}
/* this function is required by the modbus library */
uint32_t millis() {
	return systicks;
}



int isPressed(){
	int btnKeyNum =0;
	//create digi
	static DebouncedInput dInput4(4,10);
	static DebouncedInput dInput5(5,10);
	static DebouncedInput dInput6(6,10);
	static DebouncedInput dInput7(7,10);

	static bool dArray[4];

	dArray[0] = dInput4.read();
	dArray[1] = dInput5.read();
	dArray[2] = dInput6.read();
	dArray[3] = dInput7.read();

	for(int i =0;i < 4; i++){
		if(dArray[i]){
			btnKeyNum = i+1;
		}
	}

	return btnKeyNum;
}

void printRegister(ModbusMaster& node, uint16_t reg) {
	uint8_t result;
	// slave: read 16-bit registers starting at reg to RX buffer
	result = node.readHoldingRegisters(reg, 1);

	// do something with data if read is successful
	if (result == node.ku8MBSuccess)
	{
		printf("R%d=%04X\n", reg, node.getResponseBuffer(0));
	}
	else {
		printf("R%d=???\n", reg);
	}
}


/**
 * @brief	Main UART program body
 * @return	Always returns 1
 */
int main(void)
{

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
	Board_LED_Set(0, true);
#endif
#endif

	/* Set up SWO to PIO1_2 */
	Chip_SWM_MovablePortPinAssign(SWM_SWO_O, 1, 2); // Needed for SWO printf

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / 1000);

	// button registers set up
	for(int i=0; i<4;i++){
		Chip_IOCON_PinMuxSet(LPC_IOCON, dPort[i], dPin[i], (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, dPort[i], dPin[i]);
	}

	Chip_RIT_Init(LPC_RITIMER);
	NVIC_EnableIRQ(RITIMER_IRQn);
	ModbusMaster node(2); // Create modbus object that connects to slave id 2

	node.begin(9600); // set transmission rate - other parameters are set inside the object and can't be changed here


	node.writeSingleRegister(0, 0x0406); // prepare for starting

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well


	node.writeSingleRegister(0, 0x047F); // set drive to start mode


	Sleep(4000);

	LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

	lcd.begin(16,2);
	lcd.setCursor(0,0);
	SimpleMenu menu;
	DecimalEdit frequency(lcd, node, std::string("Frequency"),20000,0);
	menu.addItem(new MenuItem(frequency));
	frequency.setValue(0);

	menu.event(MenuItem::show); // display first menu item

	volatile static int k;
	while(1){
		printRegister(node, 3); // for debugging
		k = isPressed();
		if(k > 0) {
			switch(k){
			case 1:
				menu.event(MenuItem::up);
				break;
			case 2:
				menu.event(MenuItem::down);
				break;
			case 3:
				menu.event(MenuItem::back);
				break;
			case 4:
				menu.event(MenuItem::ok);
				break;
			}
		}
	}

	return 1;
}

