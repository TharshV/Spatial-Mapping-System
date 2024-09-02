# Spatial-Mapping-System
This embedded spatial measurement system uses the Texas Instruments MSP432E401Y microcontroller, a stepper motor, and the VL53L1X time of flight (ToF) sensor to acquire information and map the surrounding area. The data acquisition process begins with the user enabling the microcontroller to receive data by pressing a push button. To initiate a scan, the user would press the second push button. The stepper motor rotates to capture a y-z slice of the surrounding area. At intervals of 11.25°, distance measurements are taken as the motor rotates. Once the motor reaches 360° in rotation, it returns to its initial position by rotating in reverse. The user can decide the subsequent action of the microcontroller, whether to stop the data acquisition process with the first push button or to run another scan with the second button. Each repeated scan assumes that the device moves forward by 50 cm.
## Getting Started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.
### Prerequisites
Hardware:
- A Texas Instruments MSP432E401Y microcontroller
- VL53L1X ToF Sensor
- Push Buttons
- Jumper wires
- Breadboard
- Stepper Motor

Software:
- Keil IDE
- Any other code editor that supports Python

### Setup
Hardware:
1. Using the schematic below, build the system: ![image](https://github.com/user-attachments/assets/cbc90f20-2ff1-4db8-b83e-eb8a7030f23a)
2. Mount the ToF Sensor to the stepper method with any method of your choice
3. Mount the stepper motor in place where the ToF sensor is unobstructed while also still connected to the microcontroller

Software:
1. Clone the repo: `git clone https://github.com/TharshV/Spatial-Mapping-System.git`
