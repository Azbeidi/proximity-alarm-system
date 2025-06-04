

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <driverlib/sysctl.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include "launchpad.h"
#include "seg7.h"

// MCU connections to TiM1637's CLK and DIO pins. The default pin connections are as follows:
//	MCU Pin		Grove Pin		TiM1637 Pin
//	  PA6		  J10 SCL	      CLK
//	  PA7		  J10 SDA		  DIO
// CHANGE THE MACROS if you use a different jumper of the Grove boosterpack.
#define PORT_PERIPH		SYSCTL_PERIPH_GPIOA	// The peripheral def of Port A
#define PORT			GPIO_PORTA_BASE		// The base address of the I2C port
#define CLK				GPIO_PIN_6			// The 8-bit pin value (mask format) for clock
#define DIO				GPIO_PIN_7			// The 8-bit pin value (mask format) for data IO

// Signal timing of CLK and DIO
#define HALF_CCT		5					// Half clock cycle is 5 milliseconds
#define DELTA			1					// Time difference between CLK and DIO change, for tolerance

// Initialize the port connection to TiM1637 and the 7-segment display. TiM1637 is connected to
// the SCL and SDA pins of I2C #1, which are PA6 and PA7, respectively. However, TiM1637 is NOT I2C
// comptatible and thus we have to use PA6 and PA7 as GPIO pins (and use bit banging to emulate
// the START, STOP, and data bits of I2C).
void
Seg7Init()
{
	// Enable GPIO Port B as peripheral.
	SysCtlPeripheralEnable(PORT_PERIPH);

    // Set the pads for standard push-pull operation, with 8ma strength
    GPIOPadConfigSet(PORT, CLK | DIO, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

    // Make the pin be outputs
    GPIODirModeSet(PORT, CLK | DIO, GPIO_DIR_MODE_OUT);

    // Initial states are high
    GPIOPinWrite(PORT, CLK | DIO, CLK | DIO);
}

// Send START bit of I2C: Keep CLK high (inactive), pull DIO from high to low.
static void
tmSendStart()
{
	// Pull the CLK high for high CCT, and pull DIO high shortly after pulling CLK high
	GPIOPinWrite(PORT, CLK, CLK);
	WaitUs(DELTA);
	GPIOPinWrite(PORT, DIO, DIO);
	WaitUs(HALF_CCT - DELTA);

	// Pull the DIO low for half CCT
	GPIOPinWrite(PORT, DIO, 0);
	WaitUs(HALF_CCT);
}

static void
tmSendDataBit(bool bit)
{
	// The DIO pin value to write
	uint8_t pinValue = bit ? DIO : 0;

	// Pull CLK low, wait for DELTA time, write data bit value, then wait for a time of
	// half CCT minus DELTA
	GPIOPinWrite(PORT, CLK, 0);
	WaitUs(DELTA);
	GPIOPinWrite(PORT, DIO, pinValue);
	WaitUs(HALF_CCT - DELTA);

	// Pull CLK high, wait for half CCT
	GPIOPinWrite(PORT, CLK, CLK);
	WaitUs(HALF_CCT);
}

// Wait for ACK. The function does not really check for ACK, just wait for 1 CCT.
static void
tmWaitAck()
{
	// Pull CLK low
	GPIOPinWrite(PORT, CLK, 0);
    GPIODirModeSet(PORT, DIO, GPIO_DIR_MODE_IN);

	// Pull CLK high after half CCT
	WaitUs(HALF_CCT);
	GPIOPinWrite(PORT, CLK, CLK);

	// Change DIO to input after another half CCT
	WaitUs(HALF_CCT);
	GPIOPinWrite(PORT, CLK, 0);
    GPIODirModeSet(PORT, DIO, GPIO_DIR_MODE_OUT);
}

// Send STOP bit of I2C: Pull both low, pull DIO high in the middle of CCT
static void
tmSendStop()
{
	// Pull CLK and DIO low
	GPIOPinWrite(PORT, CLK, 0);
	WaitUs(DELTA);
	GPIOPinWrite(PORT, DIO, 0);

	// Pull CLK high after half CCT
	WaitUs(HALF_CCT - DELTA);
	GPIOPinWrite(PORT, CLK, CLK);

	// Pull DIO high after another half CCT
	WaitUs(HALF_CCT);
	GPIOPinWrite(PORT, DIO, DIO);
}

// Send a single byte (eight bits)
static void
tmSendByte(uint8_t byte)
{
	int i;
	bool bit;

	// Send the eight data bits of byte, from LSB to MSB
	for (i = 0; i < 8; i++, byte >>= 1) {
		bit = (byte & 0b00000001);
		tmSendDataBit(bit);
	}
}

// Update all 4 digits of the 7-segment displays
void
Seg7RawUpdate(uint8_t code[])
{
	tmSendStart();
	tmSendByte(0x40);
	tmWaitAck();
	tmSendStop();

	tmSendStart();
	tmSendByte(0xc0);
	tmWaitAck();
	tmSendByte(code[3]);
	tmWaitAck();
	tmSendByte(code[2]);
	tmWaitAck();
	tmSendByte(code[1]);
	tmWaitAck();
	tmSendByte(code[0]);
	tmWaitAck();
	tmSendStop();

	tmSendStart();
	tmSendByte(0x8A);
	tmWaitAck();
	tmSendStop();
}

