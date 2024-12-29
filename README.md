# Microcontrollers

This repository contains coursework, assignments, and resources for the **IKT104 Microcontrollers** subject. The course focuses on understanding and programming microcontrollers, specifically the **B-L475E-IOT01A** IoT discovery kit. It covers a wide range of topics, from basic digital and analog I/O to advanced communication protocols and operating system features.

## Course Overview

The course provides theoretical and practical knowledge of microcontroller systems, with emphasis on real-world IoT applications. The assignments and tasks explore the following key topics:

### Lecture Topics
1. **Introduction:** Basics of microcontrollers and embedded systems.
2. **Digital Inputs and Outputs:** Interfacing and controlling digital signals.
3. **Analog Inputs and Outputs:** Working with ADCs, DACs, and analog sensors.
4. **Serial and Parallel Communication:** Implementing UART, SPI, and other communication protocols.
5. **Interrupts:** Using external interrupts and watchdog timers.
6. **Timers:** Timer functionalities for scheduling and delay.
7. **Wi-Fi:** Configuring and using Wi-Fi for IoT applications.
8. **IoT Visualization:** Representing IoT data in a visual format.
9. **Mbed OS - Threads, Mutexes, and Semaphores:** Basics of RTOS and task management.
10. **Mbed OS - Event Flags, Thread Flags, and Mail:** Advanced RTOS functionalities.
11. **Logic Gates and Numeral Systems:** Introduction to digital logic.
12. **Memory Mapping and Registers:** Low-level programming of hardware.
13. **Mbed OS - I2C:** Communicating with peripherals using I2C.

## Repository Structure

The repository is organized into folders for each assignment and course delivery:

```
├── Delivery_ikt104        # Course delivery and resources
├── assignment_1           # Digital inputs and outputs
├── assignment_2           # Analog inputs and outputs
├── assignment_3           # Serial and parallel communication
├── assignment_4           # Interrupts and watchdog
├── assignment_5           # Timers
├── assignment_6           # Wi-Fi and IoT applications
├── assignment_7           # RTOS: Threads, Mutexes, and Semaphores
├── assignment_8           # I2C and advanced peripherals
├── .gitignore             # Git ignored files
├── LICENSE                # License for the repository
└── README.md              # Overview of the repository
```

## Microcontroller Used
The **B-L475E-IOT01A** IoT Discovery Kit is the primary microcontroller platform for this course. It includes:
- STM32L4 microcontroller
- Wi-Fi, Bluetooth, and various onboard sensors
- Support for Mbed OS and STM32Cube frameworks

## Purpose of this Repository
This repository serves as a centralized location for:
- **Assignments:** Practical tasks to deepen understanding of microcontroller systems.
- **Reference Materials:** Code examples, lecture notes, and documentation.
- **Project Delivery:** Submission-ready materials for evaluation.

## How to Use
1. Clone this repository:
   ```bash
   git clone https://github.com/Dogfetus/ikt104_microcontrollers.git
   ```
2. Navigate to the desired assignment folder.
3. Refer to the **README.md** or comments in the code files for specific instructions.
4. Use Mbed Studio or STM32CubeIDE to program and debug on the B-L475E-IOT01A board.

## Contact
For any questions or clarifications, feel free to contact the repository owner or refer to the course instructor.
