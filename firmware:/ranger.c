

#include "ranger.h"

//static variables

typedef struct {
    Event *callback_event;
    enum {
        IDLE,                   // ranger is idle
        EXPECT_START_EVENT_1,
        EXPECT_START_EVENT_2,
        EXPECT_DATA_EVENT_1,
        EXPECT_DATA_EVENT_2
    } state;
    bool new_data_ready;
    uint32_t measurement;
    uint32_t start_time;
} RangerState;

volatile RangerState ranger;

void RangerISR();

//sets the range of the ISR
void RangerInit()
{
    // Initial ranger state in memory
    ranger.callback_event = NULL;
    ranger.new_data_ready = false;

    // Enable GPIO Port D and Wide Timer 2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER2);

    // Doesnt enable the time as this time
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_1);
    GPIOPinConfigure(GPIO_PD1_WT2CCP1);

    // Count both edges as event
    TimerControlLevel(WTIMER2_BASE, TIMER_B, true);
    TimerControlEvent(WTIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);

    // Register an ISR to deal with the timeout event
    TimerIntRegister(WTIMER2_BASE, TIMER_B, RangerISR);
    TimerIntEnable(WTIMER2_BASE, TIMER_CAPB_EVENT);
}


void RangerTriggerReading()
{
    // Clean data ready flag and set the ranger state
    ranger.new_data_ready = false;
    ranger.state = EXPECT_START_EVENT_1;

    // It is safe to disable a peripheral during configuration
    TimerDisable(WTIMER2_BASE, TIMER_B);

    // Configure WT2CCP1 for PWM to generate a pulse of 5 microseconds, with
    // 2 microseconds of leading low time
    TimerConfigure(WTIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);
    TimerLoadSet(WTIMER2_BASE, TIMER_B, 1000);
    TimerMatchSet(WTIMER2_BASE, TIMER_B, 500);
    TimerControlEvent(WTIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
    TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);

    // Enable the timer, and start PWM waveform
    TimerEnable(WTIMER2_BASE, TIMER_B);
    //uprintf("Triggered new reading\n");
}

/*
 * If any new data is ready (for polling)
 */
bool RangerDataReady()
{
    return ranger.new_data_ready;
}

uint32_t RangerGetData()
{
    return ranger.measurement;
}


//RangerISR: Process the events in the start pulse (from Tiva C to ranger)
void RangerISR()
{
    uint32_t current_time;
    uint32_t status = TimerIntStatus(WTIMER2_BASE, true);

    switch (ranger.state)
    {
    case EXPECT_START_EVENT_1:
        TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);
        //uprintf("ISR: Start Event 1\n");
        ranger.state = EXPECT_START_EVENT_2;
        break;

    case EXPECT_START_EVENT_2:
        // Disable timer before reconfiguration
        TimerDisable(WTIMER2_BASE, TIMER_B);
        TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);

        // Reconfigure timer for capture mode
        TimerConfigure(WTIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_TIME_UP);
        TimerLoadSet(WTIMER2_BASE, TIMER_B, 0xFFFFFFFF);

        // Enable capture on both edges
        TimerControlEvent(WTIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
        TimerControlLevel(WTIMER2_BASE, TIMER_B, true);

        // Enable timer and interrupts
        TimerIntEnable(WTIMER2_BASE, TIMER_CAPB_EVENT);
        TimerEnable(WTIMER2_BASE, TIMER_B);

        //uprintf("ISR: Start Event 2\n");
        ranger.state = EXPECT_DATA_EVENT_1;
        break;

    case EXPECT_DATA_EVENT_1:
        current_time = TimerValueGet(WTIMER2_BASE, TIMER_B);
        ranger.start_time = current_time;
        TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);
        //uprintf("ISR: Data Event 1, time=%u\n", current_time);
        ranger.state = EXPECT_DATA_EVENT_2;
        break;

    case EXPECT_DATA_EVENT_2:
        current_time = TimerValueGet(WTIMER2_BASE, TIMER_B);
        ranger.measurement = current_time - ranger.start_time;
        ranger.new_data_ready = true;  // This is crucial!
        TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);
        //uprintf("ISR: Data Event 2, width=%u\n", ranger.measurement);
        ranger.state = IDLE;
        break;

    default:
        ranger.state = IDLE;
        TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);
        break;
    }
}
