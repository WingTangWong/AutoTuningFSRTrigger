# AutoTuning FSR Trigger

## Overview

This Arduino-based project provides firmware for auto-tuning Force Sensing Resistors (FSRs). It's designed primarily for applications like 3D printer Z-axis end stops and auto-bed leveling probes, but can be adapted for other pressure-sensing needs.

The codebase is currently undergoing active refactoring and enhancement to improve clarity, modularity, hardware portability, and documentation. The primary development version is based on the `Legacy/AutoTuningFSRTrigger.ino` sketch.

## Core Features (Based on `Legacy/AutoTuningFSRTrigger.ino`)

*   **Multiple FSR Inputs:** Supports 1 or 3 FSR inputs, depending on the selected board configuration.
*   **Auto-Calibration:**
    *   Performs an initial calibration routine at startup to establish baseline FSR readings.
    *   Includes a mechanism for periodic re-calibration if the system is not in a triggered state, allowing it to adapt to slow environmental changes.
*   **Digital Trigger Output:** Provides a digital output signal (HIGH/LOW) that changes state when significant pressure on an FSR is detected. By default, this emulates a Normally Closed (NC) switch suitable for many 3D printer endstop inputs.
*   **LED Status Indicators:** Utilizes LEDs to indicate the trigger status of the FSRs.
*   **Multi-Board Compatibility:** The firmware is designed to be configurable for various common Arduino-compatible microcontrollers:
    *   ATtiny85 / Adafruit Trinket (supports 1 FSR, or 3 FSRs using "X3" pin-sharing mode)
    *   ATtiny84 (supports up to 3 FSRs)
    *   Arduino Uno / ATmega328P (supports up to 3 FSRs)
    *   Arduino Mega (supports up to 3 FSRs)
*   **X3 Pin-Sharing Mode (ATtiny85):** A special mode for ATtiny85-based boards (like Adafruit Trinket) that allows three FSR inputs and corresponding LED indicators by sharing microcontroller pins. These pins dynamically switch between analog input (for FSR reading) and digital output (for LED control).

## How it Works (Briefly)

1.  **Initialization:** On startup, the system initializes pins and performs an initial calibration.
2.  **Calibration:**
    *   During calibration, the firmware reads baseline analog values from each FSR to determine a stable average resting state.
    *   It also calculates a noise level based on fluctuations in these readings.
    *   Trigger thresholds are then established based on these calibrated averages and noise levels.
3.  **Sensing Loop:**
    *   Continuously reads the current analog values from the FSRs.
    *   Compares these current readings against the calibrated thresholds.
    *   If a reading surpasses the trigger threshold (indicating pressure), the output signal changes state, and the corresponding LED is activated.
    *   The system includes logic for periodic re-calibration if it remains untriggered for a defined interval.

## Getting Started

### Hardware Requirements

*   An Arduino-compatible board (see list of supported boards above).
*   One or more Force Sensing Resistors (FSRs).
*   Pull-down resistors (e.g., 10k Ohm) for each FSR.
*   (Optional) LEDs for status indication, with appropriate current-limiting resistors if not already on the board.

### Wiring FSRs (Pull-Down Configuration)

The firmware generally assumes FSRs are wired in a voltage divider configuration using pull-down resistors:

*   Connect one terminal of the FSR to VCC (e.g., 5V or 3.3V, depending on your board).
*   Connect the other terminal of the FSR to an analog input pin on your microcontroller.
*   Connect one terminal of a pull-down resistor (e.g., 10k Ohm) to the same analog input pin.
*   Connect the other terminal of the pull-down resistor to GND.

With this setup, applying pressure to the FSR decreases its resistance, leading to a higher voltage (and thus a higher analog reading) at the ADC pin.

Refer to the `do_board_setup()` function within the `Legacy/AutoTuningFSRTrigger.ino` sketch for specific pin assignments (analog inputs, LED outputs, trigger output) for each supported board type. Detailed wiring diagrams will be added to the `/docs` folder in the future.

### Software Setup

1.  **Arduino IDE:** Download and install the latest version of the Arduino IDE.
2.  **Board Support Packages:**
    *   Ensure you have the necessary board support packages installed for your chosen microcontroller.
    *   For ATtiny85/84, this typically involves installing a core like "ATtinyCore by Spence Konde" or "arduino-tiny" via the Arduino Board Manager.
3.  **Configure Target Board in Code:**
    *   Open `Legacy/AutoTuningFSRTrigger.ino` in the Arduino IDE.
    *   Locate the `setup()` function.
    *   Modify the line `currentBoard = BoardType::ADAFRUIT_TRINKET_X3;` to select your target board from the `BoardType` enum (e.g., `BoardType::ARDUINO_UNO`, `BoardType::ATTINY85`, etc.).
4.  **Compile and Upload:** Select your board and programmer in the Arduino IDE, then compile and upload the sketch.

## FSR Information

### What Are FSRs?

Force Sensing Resistors are devices whose resistance changes when a force or pressure is applied. They typically consist of a conductive polymer that changes resistance in a predictable manner when deformed. FSRs are not precision load cells but are very useful for detecting force and pressure in a wide range of applications.
(See also: [SparkFun FSR Guide](https://www.sparkfun.com/datasheets/Sensors/Pressure/fsrguide.pdf))

### How Are FSRs Used in 3D Printers?

In 3D printing, FSRs can be placed under the print bed. When the nozzle touches the bed during probing or homing, the pressure on the FSRs changes, which can be detected by the microcontroller. This signal is then used as a Z-axis endstop or for auto-bed leveling routines, allowing the printer to compensate for uneven bed surfaces. Using FSRs can sometimes simplify effector design by removing the need for a separate mechanical probe.

## Toolchain Notes for ATtiny Microcontrollers

*   **Adafruit Trinket/Gemma:** These boards often come with a USB bootloader. Adafruit provides a customized Arduino IDE setup or instructions for adding their boards via the Board Manager, which simplifies programming.
*   **Bare ATtiny85/84 Chips:**
    *   You will typically need an ISP (In-System Programmer) like a USBasp, USBtinyISP, or an Arduino configured as an ISP to program these chips.
    *   **Fuse Settings:** For ATtiny microcontrollers, correct fuse settings are critical for proper operation, especially for clock speed. For example, to run an ATtiny85 at 8MHz using its internal oscillator (without the default /8 prescaler), specific fuse values must be programmed using a tool like `avrdude`. Refer to the `Notes/ATTiny85-Fusebits-8MhzInternalClock.md` file for an example. Incorrect fuse settings can lead to issues with timing (e.g., `delay()` being too fast or slow) or even render the chip unresponsive to programmers.

## Future Development

This codebase is being actively refactored with the following goals:

*   **Improved Modularity:** Breaking down the code into logical, reusable modules (e.g., sensor handling, calibration, hardware abstraction).
*   **Enhanced Portability:** Abstracting hardware-specific details to make it easier to support new microcontrollers. Planned targets include STM8 and STM32 families.
*   **Comprehensive Documentation:** Improving code comments, providing detailed setup guides, and creating clear API documentation where applicable.
*   **Robust Build System:** Implementing conditional compilation and build profiles for different target platforms.

## Disclaimer

This code is provided as-is. The original author and subsequent contributors take no responsibility for any potential or actual damages that may result from its use. This includes, but is not limited to, damage to 3D printers, electronics, or any other equipment. Users should test thoroughly in a safe environment. Maker beware.

## Credits and References

*   Original FSR auto-tuning concept and code by Wing Tang Wong.
*   Thanks to the DeltaBot Google Group for inspiration and discussion on FSRs in 3D printing.
*   Thanks to Rich Cattell for modifications to the Marlin firmware for auto-probing. ([RichCattell/Marlin](https://github.com/RichCattell/Marlin))
*   [SparkFun FSR Integration Guide](https://www.sparkfun.com/datasheets/Sensors/Pressure/fsrguide.pdf)
*   [Arduino Tiny Cores (for ATtiny support)](https://code.google.com/p/arduino-tiny/) (Note: This is an older link; modern alternatives like Spence Konde's ATtinyCore are widely used).

```
