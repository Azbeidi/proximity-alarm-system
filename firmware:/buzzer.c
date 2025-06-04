#include "buzzer.h"

//creating a fucntion that imports the function of the buzzer on to the portJ17 for the buzzer
void BuzzerInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5);
}
