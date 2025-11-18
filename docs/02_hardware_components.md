# 2. Robot Hardware and Components

This section provides a detailed enumeration and description of all hardware components integrated into the robot and their specifications.

## 2.1 Main Control and Processing Unit

### **Component:**  **MEGA 2560 Pro Embed**

* **Quantity:** 1
* **Voltage:** 5V (operating), 7-9V (Vin)
* **Description:** Serves as the main controller board, based on the ATmega2560 microcontroller. It processes all sensor data, executes control algorithms, and manages actuation.
* **Features:** 54 digital I/O pins, 16 analog inputs, 256 KB flash memory, and 16 MHz clock speed.

<img src="../assets/hardware_photos/mega2560pro.jpg" width="300">

## 2.2 Motion System Components


### **Component:** **Hobby Gearmotor with 48:1 gearbox**

* **Quantity:** 1
* **Voltage Range:** 3-12V (operating at ~4.2V)
* **Current Consumption** ~120 mA
* **Stall Torque:** ~1.5 kg·cm (0.15 N·m)
* **Stall Current:** ~1.6 A
* **Description:** A brushed DC motor with an integrated gearbox for increased torque. Utilized for rear-wheel propulsion.

<img src="../assets/hardware_photos/hobbymotor.jpg" width="300">

### **Component:** **Servo Motor MG90S**

* **Quantity:** 1
* **Voltage Range:** 4.8-6V
* **Current Consumption:** ~250 mA (max)
* **Torque:** 1.8 kg·cm @ 5V (0.18 N·m)
* **Description:** A micro servo motor employed for front-wheel steering. Operates via PWM signal.

<img src="../assets/hardware_photos/servo.jpg" width="300">

### **Component:** **Mini L298N Motor Driver**

* **Quantity:** 1
* **Logic Voltage:** 5V
* **Motor Voltage Range:** Up to 12V
* **Current Output:** Up to 2A per channel
* **Description:** A dual H-bridge motor driver responsible for interfacing the low-current control signals from the Arduino Mega to the higher-current requirements of the DC motors.

<img src="../assets/hardware_photos/motordriver.jpg" width="300">

## 2.3 Sensor Suite

### **Component:** **PixyCam 2.1**

* **Quantity:** 1
* **Voltage:** 5V
* **Current Consumption:** ~140 mA
* **Interface:** ICSP
* **Description:** Smart vision sensor for real-time color and object recognition. Used for obstacle detection.

<img src="../assets/hardware_photos/pixycam.jpg" width="300">

### **Component:** **TCS3472 Color Sensor**

* **Quantity:** 1
* **Voltage:** 3.3-5V
* **Current Consumption** ~235 µA (active)
* **Interface:** I2C
* **Description:** High-resolution color sensor with IR filtering. Mounted underneath the robot to detect blue and orange floor lines that indicate turns.

<img src="../assets/hardware_photos/tcs3472color.jpg" width="300">

### **Component:** **TCS3200 Color Sensor (Removed)**

* **Quantity:** 2
* **Voltage:** 2.7-5.5V
* **Power Consumption:** ~10-50 mA
* **Output:** Frequency proportional to color intensity
* **Description:** Side-mounted color recognition sensors used for detecting the magenta signal for parking. Converts color input to frequency output.

<img src="../assets/hardware_photos/tcs3200color.jpg" width="300">

### **Component:** **HC-SR04 Ultrasonic Sensors**

* **Quantity:** 4
* **Voltage:** 5V
* **Current Consumption:** ~15 mA
* **Range of Detection:** ~2 cm to ~4 m
* **Description:** Distance sensors placed at the front and rear utilized for measuring real-time distances to walls, used for collision avoidance and maintaining orientation.

<img src="../assets/hardware_photos/ultrasonicsensor.jpg" width="300">

### **Component:** **MPU6050 Accelerometer + Gyroscope**

* **Quantity:** 1
* **Voltage:** 3.3-5V
* **Current Consumption:** ~3.8 mA
* **Interface:** I2C
* **Description:** A 3-axis MPU providing acceleration and angular velocity data, used for angle correction and maintaining trajectory stability through PID control.

<img src="../assets/hardware_photos/MPU6050.jpg" width="300">

### **Component:** **Infrared Optocoupler Encoder**

* **Quantity:** 1
* **Voltage:** 3.3-5V
* **Current Consumption:** <20 mA
* **Description:** A laser interruption-based encoder used for detecting rotational position, enabling accurate measurement of distance and speed (odometry).

<img src="../assets/hardware_photos/encoder.jpg" width="300">

## 2.4 Power Supply and Regulation

### **Component:** **4.2V batteries/3.7V**

* **Quantity:** 1
* **Voltage:** 4.2V nominal
* **Capacity:** 8800 mAh
* **Discharge Current:** Up to 2A (depending on model)
* **Description:** Power supply for the robot's DC motors. Specifically, **Li-ion 18650 Battery 4.2V 8800 mAh** is used.

<img src="../assets/hardware_photos/4.2battery.jpg" width="300">

### **Component:** **Rechargable 9V Battery**

* **Quantity:** 1
* **Voltage:** 9V nominal
* **Capacity:** ~600 mAh
* **Description:** A standard 9V battery used for the sensors and Arduino MEGA unit.

<img src="../assets/hardware_photos/9Vbattery.jpg" width="300">

## 2.5 Miscellaneous Components

  ### **Component:** **Custom Chassis (RWD)**

  * **Quantity:** 1
  * **Material:** 3D printed (PLA)
  * **Description:** A 3D designed rear-wheel drive chassis designed to house motors, mounts, sensors, battery holders, and the controller.

  [**3D Modeling**](./10_3d_modeling.md)


  ### **Component:** **Pushbutton**

  * **Quantity:** 1
  * **Voltage Handling:** Signal-level (logic input)
  * **Current:** <10 mA
  * **Description:** A momentary switch used to initiate the main program.

  <img src="../assets/hardware_photos/PUSHBUTTON.jpg" width="300">

  ### **Component:** **Self-Locking Push Button Switch**

  * **Quantity:** 1
  * **Voltage Handling:** Up to ~12V
  * **Current:** Up to ~500mA
  * **Description:** A switch for toggling between two states, serving as the main power switch for the robot.

 <img src="../assets/hardware_photos/SWITCH.jpg" width="300">

  ### **Component:** **LED**

  * **Quantity:** 1
  * **Forward Voltage:** ~3V
  * **Current:** ~10-20 mA
  * **Description:** A standard blue LED used as a visual indicator, signaling program readiness.

  <img src="../assets/hardware_photos/LEDBLUE.jpg" width="300">

  ### **Component:** **Headlight LEDs**

  * **Quantity:** 2
  * **Forward Voltage:** ~3V
  * **Current:** ~10-20 mA
  * **Description:** Standard white LEDs used as headlights for the bot.

  <img src="../assets/hardware_photos/HEADLIGHTled.jpg" width="300">

  ### **Component:** **Resistor**

  * **Quantity:** 2
  * **Resistance:** 330Ω, 10kΩ
  * **Description:** Current-limiting resistor for the LED indicator 
  and pushbutton.
  
  <img src="../assets/hardware_photos/resistor.jpg" width="300">

## 2.6 Power Management  

The robot integrates a **dual-supply power architecture** to balance performance, efficiency, and protection of sensitive electronics:  

### Motor Power Supply  
* **Source:** 4.2V 18650 Li-ion Battery (8800 mAh)  
* **Load:** DC gearmotor, L298N motor driver + LED Headlights (optional)  
* **Current Draw:**  
  * ~120 mA average during motion  
  * +40 mA with headlights (2 × 20 mA)  
  * Up to ~1.6 A at stall (short bursts only)  
* **Estimated Runtime:**  
  * Continuous average draw (~120 mA): ~70 hours  
  * With headlights (~160 mA): ~55 hours  
  * Realistic mixed load with peaks: **~10–12 hours** of operation  

This battery is dedicated to propulsion and lighting, ensuring that motor noise and current surges do not affect sensor accuracy or logic circuits while maintaining long-lasting illumination for optimal operation.

### Logic and Sensor Power Supply  
* **Source:** Rechargeable 9 V Battery (~600 mAh)  
* **Load:** MEGA 2560 Pro + PixyCam + MPU6050 + Color Sensor + Ultrasonics  
* **Current Draw:**  
  * MEGA 2560 Pro: ~50–70 mA  
  * PixyCam: ~140 mA  
  * MPU6050: ~4 mA  
  * TCS3472 Color Sensor: ~0.24 mA  
  * Ultrasonic Sensors (x3): ~60 mA total  
  * **Total Average:** ~260–300 mA  
* **Estimated Runtime:**  
  * ~600 mAh / 280 mA ≈ **2 hours** continuous operation  
  * Recommended use time ≈ **1 hour** for best performance

This battery ensures clean and stable voltage delivery for sensors and the controller.  

### Voltage Regulation  
* The **MEGA 2560 Pro** regulates the 9V input to 5V for its logic and peripherals.  
* I²C devices (MPU6050, color sensors) operate safely at 3.3–5V.  

### Switching and Protection  
* **Main Switch**: Master power cutoff for the system.  
* **Pushbutton**: Starts the main program after initialization.  
* **330Ω and 10kΩ Resistor**: Ensures safe current limiting for the LED indicator and facilitates pushbutton grounding.
* **LED Indicator**: Shows program readiness.  
* **Headlight LEDs (x2)**: White LEDs for illumination during operation.  

**Summary of Power Autonomy**  
* **Motors (4.2V Li-ion):** 12–15 hours (practical runtime)  
* **Logic & Sensors (9V battery):** ~2 hours (main limiting factor)  

The system's autonomy is therefore governed by the 9V supply, after which recharging or replacement is required.

## 2.7 Circuit Design

### Original Manually Soldered Circuit

We developed a custom shield circuit for the regionals, using a soldered Printed Circuit Board (PCB) to streamline the electrical connections, in order to reduce cable clutter, and minimize the overall weight, size, and complexity of the system. All components and routes were manually soldered onto the PCB to ensure a compact and custom integration of all components.

![PCB VS Prototype Circuit](../assets/hardware_photos/imagen%20(6).jpg)

* **Detailed Electromechanical Diagram:** For a more in-depth view of how all electronic and mechanical components are interconnected in the old PCB, refer to the the electromechanical diagram:

![Electromechanical Diagram](./../schemes/electromechanical_diagram.png)

You can also visit the detailed interactive visual representation in [Interactive Cirkit Circuit Design](https://vizdrive.github.io/VizDrive_WRO2025/embeds/interactive_circuit). It's an embedded webpage hosted on GitHub Pages.

### Arduino MKR1010 Circuit PCB

![Electromechanical Diagram](./../assets/hardware_photos/MKR_circuit.png)

A version of our circuit was made for the Arduino MKR1010. The decision was made considering the potential advantages this microcontroller offers.

* **Wi-Fi and Bluetooth connectivity**: This feature facilitates debugging during the development of our robot, providing important information during the robot's execution in the track.
* **Processing Power**: The MKR1010 is powered by the SAMD21G18A — 32-bit ARM Cortex-M0+ chip, providing a 48 MHz clock speed, 32 KB SRAM (volatile memory / RAM), and excellent computing performance for demanding tasks.
* **Compact and Lightweight**: Improving our robot's sensor usage optimization, less communications and peripherals were required, which allowed ViZio to decrease clutter and weight by using the small MKR1010.

However, this version of the robot brought several limitations, encouraging us to use the old Mega 2560 Pro.

* **Limited Power Supply**: The Arduino MKR1010 is an IoT board. These types of microcontrollers promote low ambiental impact and affect the MKR1010 with its limited power supply, providing insufficient current for our components.
* **3V3 Logic**: Having a similar context from the previous point, We had to add logic shifters since the MKR1010 uses 3.3V logic, which consequently limited the amount of clutter reduced.
* **Wi-Fi Limitations**: A web hosting was developed for debugging; however, updates were delayed and slow, so Wi-Fi communication was not useful anymore since precise and updated information was required for proper debugging.

### National PCB Circuit

The new robot uses a custom KiCad PCB circuit made for the Mega 2560 Pro. We designed a personalized KiCad circuit to improve the old version, adding durability, reliability and simplicity.

![Electromechanical Diagram](./../assets/hardware_photos/UpdatedCircuitPCB.jpg)

* **Planning and Design**: Components were slightly changed for this PCB, such as using a **self-locking pushbutton switch** instead of a toggle switch. Positioning was oriented to facilitate physical construction, simplifying wire routing.
* **Track Routing**: Tracks were manually routed using two layers (rear layer and front layer).
* **Track Thickness**: To enhance current capacity, the power tracks were widened, and the ground trace was designed with greater thickness.
* **Border Thickness**: Durability was an important factor designing this PCB, so a broad border was left to protect the circuit.
* **45° Corner Tracks**: Tracks were cornered with a 45° angle, further widening the corner's thickness and enhanced rigidity while also maintaining the electromagnetic field uniformity.
* **THT Components**: THT components were preferred over SMD due to the ease of soldering, which is important for simpler builds and faster repairs.
* **GND and 5V Tracks**: Power tracks surround the PCB, reducing the complexity of the components' routing.
* **PCB Fabrication and Build**: The custom PCB was fabricated with our logo and components were soldered.

![Electromechanical Diagram](./../assets/hardware_photos/KiCad.png)

### Current PCB Circuit

* **I2C Bus Fix:** A small fix was made in the PCB circuit, the I2C bus pins (SDA, SCL) were inverted. To fix the issue with the previous version, a small pcb was soldered to reposition the pins.

You can also download the circuit files: [kicad_pcb](../src/kicad_pcb).

---

[Back to Main README.md Index](../README.md)
