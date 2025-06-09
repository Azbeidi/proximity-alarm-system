# Proximity Alarm System  
**Author:** Abdalrahman Zbeidi and Waleed Adas
**Course:** ECE 266 – Intro to Embedded Systems 
**Semester:** Spring 2025  

# Proximity Alarm System

A embedded system that uses sonar to measure distance, displays it on a 7-segment display, and triggers a buzzer if an object comes too close. Built using the TI Tiva C LaunchPad, coded in C and assembly using Code Composer Studio.

## 🔧 Features

- Real-time proximity detection using ultrasonic sensor
- Live distance feedback via 7-segment display
- Audible buzzer alarm below set threshold
- Built on Tiva C microcontroller using C and ARM assembly

## ⚙️ System Overview

### Inputs
- Ultrasonic sonar sensor (distance input)

### Outputs
- Grove buzzer (sound alarm)
- 7-segment display (live distance readout)

### Functions
- Poll sonar sensor for object distance
- Convert and display distance on 7-segment
- Trigger buzzer if distance is below configured threshold


## 🧱 Hardware

| Component            | Description                                          |
|----------------------|------------------------------------------------------|
| Sonar Sensor         | Sends echo-based distance to Tiva C                  |
| Tiva C Microcontroller | Core processor handling all input/output logic     |
| 7-Segment Display    | Displays current distance                            |
| Grove Buzzer         | Audible alarm when object is detected too close      |
| USB Power            | Power and programming via USB to LaunchPad           |

### Software Modules

- `sonar.c`: Sends and reads sonar pulses, computes distance
- `display.c`: Updates 7-segment with formatted output
- `buzzer.c`: Triggers buzzer based on distance threshold
- `main.c`: Handles loop logic, timing, and module calls

## 🧪 Demo Summary

System responded with sub-second latency. Distance updated in real time on display, and buzzer triggered when object entered the preset danger zone.

**Memory Usage:**
- Flash: 19,065 bytes (7%)
- SRAM: 6,116 bytes (18%)

## 📁 Repo Contents

- `firmware/` – Embedded C + assembly source code
- `docs/` – Project report

