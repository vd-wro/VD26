# 11. Other Resources

This section provides links to supplementary materials, diagrams, and other relevant resources that offer further insight into the VizDrive robot's design, construction, and operation.

This document is primarily intended to serve as a **user manual**, with the set of instructions to build, calibrate, and operate our robot.

## 11.1 3D Models

All custom mechanical parts designed for VizDrive are available as STL files. 
These models can be used for replication, modification, or further study of the robot's physical structure.

* **Chassis:** The main structural frame of the robot.
  * [View 3D Model (STL)](./../models/chassis/chassis.stl)
  * [View Updated 3D Model (STL)](./../models/chassis/chassis2.0.stl)
  * [View International 3D Model (STL)](./../models/chassis/ViZioInternationalMotorFix.stl)
* **Front Wheel Rims:** Custom-designed rims for the front wheels.
  * [View 3D Model (STL)](./../models/wheels/wheel_rims.stl)
* **Front Wheel Hubs with Screws:** Hubs connecting to the steering rod, including screw designs.
  * [View 3D Model (STL)](./../models/wheels/wheel_hub.stl)
* **Encoder Wheel and Rear Wheel:** Precision-designed wheel with encoder markings and standard rear wheel.
  * [View 3D Model (STL)](./../models/wheels/encoder_wheel.stl)
* **Steering Rods and Camera Support:** Components for steering rods and the PixyCam mount.
  * [View 3D Model (STL)](./../models/steering/steering_rods.stl)
 
For the 3D printing guide, visit the [3D Modeling](./10_3d_modeling.md) document. It explains all the parameters and configuration we used for the Creality Ender 3 3D printer.

Before the fabrication of the robot, you can also visualize the **Unity simulator** we created during the modeling process, which is available as a GitHub pages hosted web embed here: [Unity Simulator](.././embeds/Unity_simulator/README.md). Click on the image to access the **Unity web player**.

[![Unity Simulator Web Player](.././embeds/Unity_simulator/Unity_simulator_web_capture.png)](https://vizdrive.github.io/VizDrive_WRO2025/embeds/Unity_simulator/)

## 11.2 Components and Wiring

For the construction of our robot, we tried to utilize the most accessible sensors and actuators. Most of them are typically found in standard Arduino kits.

To get the list of all components, refer to the [**Hardware Components**](02_hardware_components.md) document. Which also details on their specifications and utilization.
And for a look into the pin configuration and interface, refer to the [**Sensors and Pin Configuration**](04_sensors_and_pin_configuration.md) document.

For the wiring, we created a custom PCB design. All the connections are traced in the **Electromechanical Diagram** made with Cirkit. 
Which also has an [**interactive web design**](https://vizdrive.github.io/VizDrive_WRO2025/embeds/interactive_circuit), created with an HTML embed. It also includes a ChatBot that could help you to understand the general operation of the circuit and its components' connection.

![Electromechanical Diagram](./../schemes/electromechanical_diagram.png)

We've also created a custom KiCad PCB to improve durability, reliability and simplicity. You can acces the files in [kicad_pcb](../src/kicad_pcb).

## 11.3 Building Instructions

We created a 3D animation that showcases the detailed building instructions for all the components into our robot's 3D models.
The video also includes the use of different screws and nuts to ensure the correct tolerance and orientation for a perfect functioning.

For an in-depth explanation into this and other concepts taken into consideration during the planification of 3D parts, you can visit the [3D Modeling](./10_3d_modeling.md) document, which details the principles used during the construction of our robot.

[![Building Instructions](https://img.youtube.com/vi/Cte4G7Wkx94/0.jpg)](https://www.youtube.com/watch?v=Cte4G7Wkx94)

[![ViZio 2.0](https://img.youtube.com/vi/JOWvUNmzN1w/0.jpg)](https://www.youtube.com/watch?v=JOWvUNmzN1w)

## 11.4 Sensors' Calibration

### MPU-6050 Gyroscope + Accelerometer

The **MPU-6050** was calibrated by calculating the offset through averaging a specific number of samples. The empirical data collected during MPU calibration, demonstrating the reduction in accumulated error as the number of samples increases, can be visualized in the graph below.

![Calibration Data Graph (PNG)](./../assets/data_graphs/MPU_data_graph.png)

This **Accumulated Error vs. Number of Samples** graph indicates that 250 samples are sufficient to achieve an accuracy of 
±0.01 in our sensor model. However, this may vary depending on the specific gyroscope and accelerometer model.

We provide our calibration code, which calculates the offset accumulation of the MPU-6050 over a defined period, making the necessary data analysis for calibration easily accessible. When using this code, ensure the MPU-6050 remains static.

For more information on MPU-6050 orientation control, please refer to the [**PID Gyroscope Control**](06_pid_gyroscope_control.md) document.

### HC-SR04 Ultrasonic Sensor

The HC-SR04 ultrasonic sensor required an algorithm to mitigate noise and ensure more accurate measurements. We developed a custom median and outlier filter for this purpose. The **Raw vs. Filtered Ultrasonic Measurement Graph** illustrates the difference between the raw and filtered ultrasonic measurements.

![View Filter Data Graph (PNG)](./../assets/data_graphs/ultrasonic_data_graph.png)

It is crucial to evaluate the optimal number of samples for the filter to maintain consistent measurements without impacting performance. We highly recommend using **5 samples** for the filter, but also provide the [**ultrasonic sensor median filter code**](.././src/ultrasonic_median_filter/ultrasonic_median_filter.ino) if you want to visualize your own data.

For more information on HC-SR04 ultrasonic measurement and filter, please refer to the [**Ultrasonic Sensing**](08_ultrasonic_distance_sensing.md) document.

## 11.5 PID Control Parameters

The PID Control parameters are empirically tuned following the next set of steps:

* **Kp (Proportional Gain)**: is adjusted by gradually increasing it until the robot starts oscillating around the desired path (indicating overcorrection), then slightly reducing it to achieve a balanced response.
* **Kd (Derivative Gain)**: is tuned after Kp to smooth the response, reduce overshoot, and dampen oscillations, leading to a more precise and stable trajectory.
* **Ki (Integral Gain)**: is used to correct accumulative errors. It is not implemented in the code due to over-correction and wind-up (more information on this topic).

We highly recommend visiting our documentation on this topic, to understand the PID control parameters adjustment and the **Ki parameter** exclusion [**PID Gyroscope Control**](06_pid_gyroscope_control.md)

## 11.6 Code Structure

The most recent and complete version of our code is located in the `./src/main_control` folder. This code is optimally organized into various modules with `.cpp` and `.h` files, alongside a `main.ino` file. However, for presentation and readability, it's provided as a single, functional code.

While code modularity is an optional step for more sophisticated and larger systems, for simplicity, the different sections are clearly separated by comments within the code itself.
For more information, refer to our documentation on the [**Software Architecture**](03_software_architecture.md).

## 11.7 Evolution and Improvements Made Within the Versions ViZio

*Ever wondered where ViZio came from? It came from "Viz", part of our name "Vizdrive" and "io", commonly standing for input/output in computer science, which refers to how a computer system communicates with the outside world. It also happens to sound like “vicio” in Spanish, meaning a strong habit or obsession, and, well, somos unos viciados en la Robótica.*

| ViZio | ViZio 2.0 | ViZio International |
|:-----------:|:-----------:|:-----------:|
|<img src="./../assets/hardware_photos/imagen (26).jpg" height="150">|<img src="./../assets/hardware_photos/ViZio_2.0.png" height="150">|<img src="./../assets/hardware_photos/ViZioV3.png" height="150">|

### ViZio - Regional Events

First version of our robot; it had plenty of flaws but was a solid machine. Some components were held by rubber bands, others with glue, and some with screws. It had double front ultrasonic sensors, an add-on bumper, side color sensors, and a manually soldered PCB. Some general defects this version had:

* **Double Front Sonars**: This was first thought to be used when turning while avoiding an obstacle, it indicated when the turn was complete by detecting the distance to the wall. The 45° angle gave the ultrasonic sensor the perfect tilt to detect the wall when the robot was steering. This was later removed to simplify ViZio and improve driving algorithms.
  
  <img src="./../assets/hardware_photos/imagen (27).jpg" height="150">

* **Rear Ultrasonic Sensors**: The rear ultrasonic sensors were positioned very close to the robot to reduce overall length. However, combined with interference from the wheels, this placement caused unstable measurements due to sound reflections from the rear wheels. On more recent versions, ultrasonic sensors were better positioned.
  
  <img src="./../assets/hardware_photos/imagen (30).jpg" height="200">

* **Manually Soldered PCB**: The robot had a custom made PCB soldered by us. It was used to eliminate potential clutter from using breadboards—though using breadboards wasn't even a considered option, since it wouldn't fit into the sleek design of our robot. Nevertheless, manually soldered wires were constantly damaged and this design was unreliable.

  <img src="./../assets/hardware_photos/imagen (22).jpg" height="150">

* **Side Color Sensors**: Side-mounted TCS color sensors were installed primarily to detect the magenta parking walls. They were also intended to identify blocks in cases where the robot approached them from the wrong direction.

  <img src="./../v-photos/right_side.jpg" height="150">

* **Faith Over a Screw**: We were *very well known* from our flying pcb during the regional events... We bet this was ***not the best idea*** 😄.

  <img src="./../v-photos/left_side.jpg" height="150">

### ViZio 2.0 - National Event

The second version of our robot, with plenty improvements, mostly mechanical, but we still worked over software improvements. These are the main improvements made to the robot, for more information please visit [**Robot Mobility**](./05_robot_mobility.md) and [**Hardware Components**](./02_hardware_components.md).

* **Simpler Build**: Simplicity is crucial, we removed unused components: side color sensors and angled ultrasonic sensors.

* **Parallel Parking**: Parallel parking was implemented using encoders instead of color sensors.

* **Built-in Bumper**: The bumper was implemented on the previous version, but as an add-on, for this version, a built-in bumper was implemented.

* **Servo Motor Support**: A secure servo mount was implemented to acquire steering stability and reduce possible errors. The caster angle implemented is working in conjunction with this system to achieve precise maneuvers.

* **Ultrasonic Sensor Mount**: Snug-fit screwless mounts allow fast repairs and simpler building during competition. Tolerance management principles were applied to fine tune friction between the chassis and the sonars.

* **LED Headlights**: Additional illumination improves camera readings during low light scenarios. The PixyCam's built-in LEDs enhance lighting and supports the robot's headlights.

* **Chassis Redesign**: The redesigned chassis has an array of benefits during the construction phase, not only facilitating its assembly, but also improving printing time, quality, and durability by using a flat base.
 
* **Lifted Chassis**: During rounds we noticed mats were not completely flat, so we decided to lift the car, making the robot capable to traverse bumps .

* **Double Camera Mount**: During the development of ViZio, multiple camera angles were tested to identify the best view. However, camera movement was restricted to a single fixed slot; hence, to further amplify flexibility, dual camera mounts were implemented into our new model.

* **Realignment**: To realign the bot, we programmed it to bump itself with the outer wall four times every lap.

* **KiCad PCB**: ***No more flying screw...*** We designed a custom KiCad PCB to avoid all problems mentioned from the first version!

![ViZio 2.0](./../assets/hardware_photos/ViZio_Photo0.png)

### ViZio V3 - International Event

![ViZio V3 Logo](./../assets/gif_animations/V3Logo.png)

![ViZio V3](./../assets/hardware_photos/ViZioV3b.jpg)

This is our latest version, with a few final touches mostly over the software to avoid crashes and optimize the system:

* **Faster Robot**: Driving algorithms and routes were optimized to achieve faster lap times. Instead of realigning the robot with the wall after every turn, realignment is now performed only once per lap.

* **Different Turn Maneuvers**: The robot uses three distinct turning maneuvers, selected based on its distance from the wall: close, mid-range, and far.

* **Pivoting Maneuvers**: There are also three pivoting maneuvers executed after obstacle avoidance. The appropriate maneuver is chosen according to the robot’s yaw error.

* **Soldered MPU**: Performance improved significantly by soldering the MPU and avoiding excessive vibrations.

## 11.8 Technical Issues and Implemented Remedies

* **Safe Delay Looping**: This was one of the toughest problems we faced. The robot looped during the `safeDelay()` functions infinitely. Initially we thought it was a `millis()` or `micros()` saturation problem, but this was not the root cause of the problem. After debugging we found out the MPU stopped working arbitrarily, which froze the `safeDelay()` functions that used `updateOrientation()`. Two things were implemented for the solution:
  *  First, a `micros()` control was used to update MPU values with a controlled frequency. This reduced the casualties of looping but did not fully eliminate the problem.
  *  Secondly, a `Wire.setWireTimeout(3000, true)` to reset the bus anytime no I2C information was received from the gyroscope, this allowed the bus to restart and keep receiving orientation data after it freezes.
 
## 11.9 Important Notes

* **Wheel Encoder Paint**: It’s important to paint the encoder wheel as shown in [**3D Modeling**](./10_3d_modeling.md) to ensure proper operation of the light-based encoder. Otherwise, light may pass through the thin plastic layer.

* **Ultrasonic Sensors**: Before installation, it’s needed to carefully straighten the ultrasonic sensor pins with pliers. When sliding the sensor into its friction-fit mount, only the left and right units should be secured with wire to ensure proper alignment and operation.

* **Pixy Calibration**: It’s crucial to cover the Pixy’s lens with your finger during initialization to calibrate its brightness and avoid variable exposition due to slight lighting changes during power-up.

---

If you notice any inconsistency on this code functionality, feel free to contact us via email: **<vizdrive.wro@gmail.com>**!
Of course, we are also open to any recommendation, inquiry or comment on any topic.

[Back to Main README.md Index](../README.md)
