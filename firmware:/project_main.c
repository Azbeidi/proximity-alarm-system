#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "launchpad.h"
#include "seg7.h"
#include "ranger.h"
#include "buzzer.h"


// System state
typedef struct {
    enum {Millimeter, Inch} display_mode;       // display inch or millimeter
} SysState;

// The events
Event trigger_ranger_reading_event;
Event toggle_mode_event;

SysState sys = {Millimeter};

void TriggerRangerReading(Event *event)
{
    static uint32_t last_trigger_time = 0;
    uint32_t current_time = event->time;

    // Only trigger a new reading if enough time has passed
    if (current_time - last_trigger_time >= 500) {
        RangerTriggerReading();
        last_trigger_time = current_time;
    }

    // Check if new data is ready
    if (RangerDataReady()) {
        uint32_t pulse_width = RangerGetData();

        // Print raw pulse width in clock cycles
        uprintf("Pulse Width: %u cycles\n", pulse_width);

        // Convert pulse width to distance (in mm)
        float distance_mm = (pulse_width / 50000000.0) * 340.0 / 2 * 1000;
        float distance_inch = distance_mm / 25.4;

        // Buzzer control based on distance
        if (distance_mm < 475) {
            BuzzerOn();
        } else {
            BuzzerOff();
        }

        // Update the display based on the current mode
        Seg7Display seg7_display = {0};

        if (sys.display_mode == Millimeter) {
            int distance = (int)distance_mm;
            seg7_display.digit[0] = (distance % 10);
            seg7_display.digit[1] = ((distance / 10) % 10);
            seg7_display.digit[2] = ((distance / 100) % 10);
            seg7_display.digit[3] = ((distance / 1000) % 10);
            seg7_display.colon_on = false;

        } else if (sys.display_mode == Inch) {
            int feet = (int)(distance_inch / 12);
            int inches = (int)(distance_inch) % 12;

            seg7_display.digit[0] = inches % 10;
            seg7_display.digit[1] = inches / 10;
            seg7_display.digit[2] = feet % 10;
            seg7_display.digit[3] = feet / 10;
            seg7_display.colon_on = true;
        }

        Seg7Update(&seg7_display);
    }

    // Schedule next check
    EventSchedule(event, event->time + 100);  // Check every 100ms
}

// Toggle display mode between Millimeter and Inch
void ToggleMode(Event *event)
{
    if (sys.display_mode == Millimeter) {
        sys.display_mode = Inch;
    } else {
        sys.display_mode = Millimeter;
    }

    // Schedule next toggle
    EventSchedule(event, event->time + 2000); // Toggle every 2 seconds
}

//main
void main(void)
{
    // Initialize the LaunchPad and peripherals
    LaunchPadInit();
    RangerInit();
    Seg7Init();
    BuzzerInit();

    // Initialize the events
    EventInit(&trigger_ranger_reading_event, TriggerRangerReading);
    EventInit(&toggle_mode_event, ToggleMode);

    // Schedule time event
    EventSchedule(&trigger_ranger_reading_event, 100);
    EventSchedule(&toggle_mode_event, 2000);


    // Loop forever
    while (true)
    {
        asm("   wfi");       // Wait for interrupt
        EventExecute();      // Execute scheduled callbacks
    }
}
