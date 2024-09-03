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
4. Connect microcontroller to a computer
5. Go to Device Manager (on Windows) and search for Serial Connections. Take note of the COM # of the connection that says UART

Software:
1. Clone the repo: `git clone https://github.com/TharshV/Spatial-Mapping-System.git`
2. Install Python and required packages:
   - `pip install pyserial`
   - `pip install open3d`
4. Install Keil uVision 5 and open:
   1. Within Keil open: `main.c`
   2. Click on the options for Target 1 (magic wand icon) and ensure that the ARM compiler is set to default:
      ![image](https://github.com/user-attachments/assets/cd786c94-dd92-4a35-bc2a-0accd242d21e)
   3. Go to the Debug tab in the Target 1 Options and ensure that the CMSIS Debugger is used:
      ![image](https://github.com/user-attachments/assets/b7c481ee-f65b-4d28-ad22-e7c19b91f7f2)
   4. Exit target 1 Options menu
   5. Load code onto microcontroller. To do this, in order click:
      1. Translate: ![image](https://github.com/user-attachments/assets/c7a1232d-5636-40c5-9dfa-4d04c26933be)
      2. Build: ![image](https://github.com/user-attachments/assets/e03c5a67-81df-4fa8-bb9f-cdc55da6f49c)
      3. Load: ![image](https://github.com/user-attachments/assets/4be1ce1a-be41-4815-a964-527a192bf60c)
5. Click the reset button on the microcontroller.
6. Within `Data_Acquisition_Visualization.py` change `COM5` in line 48 to the COM# noted in Step 5
7. Run the python script: `python Data_Acquisition_Visualization.py`
   - Press Enter when prompted
8. Use the buttons to start, pause, or stop a measurement
   - Use button 0 (connected to PM0) to start or stop a scan
   - Use button 1 (connected to PM1) to pause during a measurement
9. After 1 full 360° rotation, the motor rotates 360° in the opposite direction to reset its position.
10. Once the measurement is stopped, a graph of dots will appear in a new window. Exit the window to view the full 3D model




   
