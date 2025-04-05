# ESP32 CKP Signal Generator

A project designed to generate CKP and CMP signals for bench-testing an ECU. This project utilizes an ESP32 microcontroller with the ESP-IDF framework, integrates an HD44780-based LCD display, and features three control buttons for user interaction.

---

## Features

- **User Interaction:**  
  - **Buttons:** Up, down, and confirm buttons to select synchronization mode.
  - **Potentiometer:** Adjust RPM control in real time.
  - **LCD Display:** Display syncronism selection menu and current RPM.

- **Output Signals:**  
  - **CMP and CKP (5V):** Simulates sensor outputs using a Hall effect sensor.
  - **Square Wave CKP:** Simulates an inductive sensor.

- **Power Supply:** 
  Uses a external 12V supply for the square wave output, other components is supplied with 5v using a LM7805.

- **Complete Schematics:**  
  All necessary schematics for building the hardware are provided with the project.


---

## Setup and Installation

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/LucasStraps/ESP32-CKP-Signal-Generator.git
   ```

2. **Open in VSCode:**
   - Launch VSCode.
   - Open the project folder.

3. **Install ESP-IDF Plugin:**
   - If not already installed, add the ESP-IDF extension from the VSCode marketplace.
   - Follow the setup instructions provided by the extension.

4. **Build and Flash:**
   - Use the ESP-IDF plugin commands to build the project.
   - Flash the firmware to your ESP32 board.
   - The signal is generated constantly on core 0, so ignore the watchdog error. You can disable it on SDK configuration.

---

## Operation

1. **Signal Generation Process:**
   - The LCD display will present options for synchronization selection.
   - Use the **up** and **down** buttons to choose the desired synchronization mode.
   - Press the **confirm** button to start generating the CKP and CMP signals.
   - Adjust the RPM using the potentiometer as needed.
   - To restart or select a different synchronization mode, press any button.

2. **Signal Outputs:**
   - **CMP & CKP (5V):** Outputs simulate a Hall effect sensor.
   - **Inductive CKP:** Provides a square wave output to simulate an inductive sensor, as the ECU only detects when the wave passes the 0v mark, the wave format does not matter, so it does not neet to be a sine wave.

---

## Schematic

All schematics for the hardware connections and design are provided within the project repository. Refer to the `schematic/` directory for detailed diagrams and connection guides.

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---
