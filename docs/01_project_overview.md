# 1. Project Overview

## 1.1 Introduction to VizDrive 2025

ViZio is an autonomous robot developed by a Panamanian 🔴⚪🔵 Team VizDrive for the **2025 World Robot Olympiad (WRO)**.
Our project participates in the **Future Engineers** category. Our goal is to apply advanced robotics, data analysis, error management, and optimization principles to represent Panama in the international field of robotics.

![Vizdrive Overview](../assets/hardware_photos/ViZio_2.0_Lights.JPG)

The primary objective of ViZio is to autonomously navigate a predefined closed-loop circuit, and to detect and evade obstacles. This is achieved using a combination of artificial vision for obstacle detection, and an array of sensors for decision-making. Every single part of the robot is designed, optimized, and constructed by the VizDrive Team.

In the Future Engineers category, we compete in two distinct challenge rounds: the **Open Challenge** and the **Obstacle Challenge**. While the robot shares most of its core functionalities across both rounds, there are key adjustments between the two. For clarity, we present the robot as a unified system in this documentation, and highlight the specific differences as they arise.

## 1.2 Main Components

| Component | Function |
| ---------------- | ------------------------------------------------------------------------------------------------------------------------ |
| **DC Motor** | Employed for rear-wheel drive (RWD) propulsion. |
| **Wheel Encoder** | Measures the rotational pulses per revolution (PPR) of the wheels, used for odometry and precise maneuvers. |
| **Servo Motor** | Utilized for steering control of the front wheels. |
| **PixyCam 2.1** | Responsible for obstacle recognition and color-based detection. |
| **Ultrasonic Sensors** | Measures real-time distance to walls, used for turns and collision avoidance. |
| **MPU Gyroscope with PID Control** | Provides continuous orientation data, which is processed by a Proportional-Integral-Derivative (PID) controller, to ensure stable and straight trajectory. |
| **Color Sensor** | Employed for detecting specific colored lines marked on the circuit, signaling turns. |

Each of these parts have their own separated document, delving into their calibration, common errors, logic and function in the robot.

## 1.3 Dimensions

| Specification | Information |
| ---------------- | ---------- |
| Height | 13 cm |
| Width | 14 cm |
| Length | 26 cm |
| Weight | 606 g |

---

## 1.4 Main Operational WorkFlow

The robot's operation can be divided into three phases:

### 1. **Initialization Phase (Setup):**

* **Power-Up**, **Self-Checks** and **Calibration:** upon power-up, the robot ensures all sensors and actuators are operational, and calibrates the MPU gyroscope yaw angle for orientation. If a sensor is not initialized correctly, the program will not start.
* **Waiting for Start:** when the robot is ready for operation an LED indicator illuminates to signal readiness, and the program starts upon a button press.

### 2. **Autonomous Navigation Phase:**

This is the primary operational phase, a loop where the robot continuously checks and adapts to its environment:

* **Orientation Correction (Continuous):**
  * The **MPU-6050** provides real-time orientation data.
  * This data is fed into a dedicated PID (Proportional-Integral-Derivative) controller, which compensates for any rotational drift, ensuring the robot maintains a stable and straight heading throughout its trajectory.
  * To correct the **MPU-6050's accumulated offset**, in the **Obstacle Challenge**, the robot uses the rear wall to **reset its yaw angle**. On the other hand, in the **Open Challenge**, it uses the robot's **parallel distance to the wall**.
* **Obstacle Detection and Evasion (Conditional):**
  * While maintaining trajectory, the **PixyCam 2.1** constantly scans the path ahead for obstacles.
  * If an obstacle is detected within the predefined **Region of Interest (ROI)** of the camera's field of view:
    * **Evasion Maneuver:** the robot turns until the obstacle is no longer detected within the PixyCam's ROI. The objects enter a "safe area" depending on the color, which indicates s obstacles are already evaded.
    * **Re-acquisition of Trajectory:** following an evasion maneuver, the robot re-establishes its MPU-based trajectory correction. Concurrently, side ultrasonic sensors are utilized to assist in re-centering the robot on its intended path.

### 3. **Completion Phase:**

* The robot transitions to this phase after completing the required number of laps.
  * **Parking Maneuver:** the robot initiates a maneuver to arrive to the parking lot; afterwards, it executes a pre-calculated parking maneuver to conclude the challenge.

---

## 1.5 General Workflow Diagram

Made with Mermaid for GitHub. Visit [Workflow Diagram.png](./../assets/flowcharts/flowchart.png) if the image is distorted or not seen.

```mermaid
graph TD
    %% Nodes
    Start([Start / Power On]) --> Setup[Setup: Init MPU6050, Pixy, Motors]
    Setup --> Calib[Calibrate Gyroscope Offset]
    Calib --> Button{Start Button<br>Pressed?}
    
    Button -- Yes --> ExitPark[Exit Parking Zone]
    ExitPark --> Loop(Start Main Loop ~33Hz)

    subgraph "Navigation & Control Loop"
        Loop --> Sense --> PID[PID Control: Maintain Yaw]
        PID --> ParkMode{Parking Mode<br>Active?<br>}
        
        ParkMode -- Yes --> ParkMan[[Parallel Parking Routine]]
        ParkMan --> End([End Program])
        
        ParkMode -- No --> ColorCheck{Floor Color<br>Detected?}
        
        ColorCheck -- Yes (Blue/Orange) --> TurnLogic[[Corner Turn Routine]]
        TurnLogic --> UpdateLap[Update Lap Count]
        
        ColorCheck -- No --> ObsCheck{Obstacle in<br>ROI?}
        
        ObsCheck -- Yes (Red/Green) --> EvadeLogic[[Obstacle Evasion Routine]]
        EvadeLogic --> Recenter[Recenter Trajectory]
        Recenter --> Loop
        
        ObsCheck -- No --> Drive[Drive Forward]
        Drive --> Loop
    end

    %% Styling
    style Start fill:#90EE90,stroke:#333,stroke-width:2px,color:black
    style End fill:#FF6961,stroke:#333,stroke-width:2px,color:black
    style Button fill:#FFD700,stroke:#333,stroke-width:2px,color:black
    style Loop fill:#C1E1C1,stroke:#333,stroke-width:2px,color:black
    style ParkMode fill:#FFD700,stroke:#333,stroke-width:2px,color:black
    style ColorCheck fill:#FFD700,stroke:#333,stroke-width:2px,color:black
    style ObsCheck fill:#FFD700,stroke:#333,stroke-width:2px,color:black
    
    style Setup fill:#ADD8E6,stroke:#333,stroke-width:1px,color:black
    style Calib fill:#ADD8E6,stroke:#333,stroke-width:1px,color:black
    style ExitPark fill:#ADD8E6,stroke:#333,stroke-width:1px,color:black
    style Sense fill:#adcae6,stroke:#333,stroke-width:1px,color:black
    style Drive fill:#ADD8E6,stroke:#333,stroke-width:1px,color:black
    style PID fill:#ADD8E6,stroke:#333,stroke-width:1px,color:black
    style UpdateLap fill:#ADD8E6,stroke:#333,stroke-width:2px,color:black
    style Recenter fill:#FF6961,stroke:#333,stroke-width:2px,color:black
    
    style ParkMan fill:#C1E1C1,stroke:#333,stroke-width:2px,stroke-dasharray: 5 5,color:black
    style TurnLogic fill:#C1E1C1,stroke:#333,stroke-width:2px,stroke-dasharray: 5 5,color:black
    style EvadeLogic fill:#C1E1C1,stroke:#333,stroke-width:2px,stroke-dasharray: 5 5,color:black

```

```mermaid
graph TD
    A[Detect Floor Color] --> B{Is it Lap 0?}
    B -- Yes --> C[Define Direction:<br>Blue=Left, Orange=Right]
    B -- No --> D[Use Pre-defined Direction]
    C & D --> E[Measure Front Distance]
    E --> F{Wall Close?}
    F -- Yes --> G[Tight Maneuver:<br>Back up & Turn]
    F -- No --> H[Open Maneuver:<br>Smooth Turn]
    G & H --> I[Update Target Yaw <br>+/- 90 deg]
    
    %% Styling
    style A fill:#ADD8E6,color:black
    style B fill:#FFD700,color:black
    style C fill:#FF6961,color:black
    style D fill:#C1E1C1,color:black
    style E fill:#ADD8E6,color:black
    style F fill:#FFD700,color:black
    style G fill:#FFB347,color:black
    style H fill:#FFB347,color:black
    style I fill:#90EE90,color:black
```

```mermaid
graph TD
    E_A["Detect Blocks with Pixy"] --> E_B{"Color?"}
    E_B -- Rojo --> E_C["Avoid to the RIGHT"]
    E_B -- Verde --> E_D["Avoid to the LEFT"]
    E_C --> E_E["Set Direction of Servomotor"]
    E_D --> E_E
    E_E --> E_F["Wait untill the block leaves vision"]
    E_F --> E_G["Return to the trajectory"]

    %% Styling
    style E_A fill:#ADD8E6,color:black
    style E_C fill:#DC143C,color:black
    style E_D fill:#008000,color:black
    style E_E fill:#FFB347,color:black
    style E_F fill:#ADD8E6,color:black
    style E_G fill:#90EE90,color:black
```

[Back to Main README.md Index](../README.md)
