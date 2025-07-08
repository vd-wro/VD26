# 3. Software Architecture

This section details VizDrive's software architecture, covering essential **libraries**, an overview of its **modular functionalities**, and the overall **firmware structure**. The code is implemented in **C++** for the Arduino Mega 2560 Pro Embed.

---

## 3.1 Required Libraries

VizDrive uses the following Arduino and Adafruit libraries to enable its functionalities:

* **`Servo.h`**: Standard Arduino library for controlling servo motors, specifically for steering.
* **`Wire.h`**: Facilitates I2C (Inter-Integrated Circuit) communication, used by the MPU6050 and the I2C color sensor.
* **`NewPing.h`**: Dedicated library for ultrasonic sensors, optimizing distance measurements.
* **`Adafruit_MPU6050.h`**: Provides the interface for the MPU6050 gyroscope and accelerometer.
* **`Adafruit_Sensor.h`**: A core library supporting various Adafruit sensors, including the MPU6050.
* **`Adafruit_TCS34725.h`**: Integrates the TCS3472 I2C color sensor (this library is compatible with both TCS3472 and TCS34725 color sensor).
* **`Pixy2.h`**: Enables communication and data retrieval from the PixyCam 2.1.

---

## 3.2 Modular Functionalities

The firmware is structured into independent modules, each abstracting a primary robot function. This modular design improves readability, simplifies debugging, and supports isolated development of specific code sections.

This document provides an **overview** of each module. We highly recommend referring to each module's dedicated documentation for a comprehensive explanation of functionalities.

### Motion and Steering

This module directs the robot's physical movement, including propulsion and directional control.

* **`driveForward(speed)`**: PWM-controlled function for DC motor operation, setting speed as a PWM value (0-255).
* **`driveBackward(speed)`**: PWM-controlled function for DC motor operation, setting speed as a PWM value (0-255) to move the robot in reverse.
* **`stopMotors()`**: Implements a "soft stop" by disabling motor PWM signals, allowing for natural deceleration.
* **`stopBrake()`**: Executes a "hard stop" using short-circuit braking for immediate halts.
* **`setSteeringAngle(angle)`**: Adjusts the front-wheel steering servo position. Angle values are bounded by `SERVO_LEFT` and `SERVO_RIGHT` constants to prevent mechanical interference.
* **`performParking()`**: Manages the precise reverse maneuver required for parking, utilizing encoder pulse count for distance control.
* **`encoderISR()`**: An Interrupt Service Routine (ISR) that increments an internal counter (`encoderPulseCount`) for accurate distance tracking.

**Key Constants:**

* `SERVO_LEFT`, `SERVO_RIGHT`: Define the minimum and maximum safe steering angles.
* `MOTOR_INA_PWM`, `MOTOR_INB_PWM`: Specify the PWM pins for controlling the H-bridge motor driver.
* `PARKING_PULSES`: The calibrated encoder pulse count for the parking distance.

For detailed explanations of these functions, refer to [**Robot Mobility**](./05_robot_mobility.md).

---

### Distance Detection

This module employs ultrasonic sensors to measure real-time distances to surrounding environmental features, primarily for maintaining optimal positioning relative to walls.

* **`getDistance(sonar)`**: Calculates and filters distance measurements using a `NewPing` object. It performs multiple pings and applies a median-based filtering process to reduce noise and improve accuracy, considering only stable and valid readings within a defined range.

* **`avoidWall(correctionAmount)`**: In the **Open Challenge**, this function adjusts the robot’s orientation by modifying its target yaw when the distance to nearby walls falls outside expected thresholds. It uses the `correctionCooldown()` mechanism to prevent repeated or excessive corrections. The correction thresholds are tuned to keep the robot aligned near the outer wall, allowing adaptability to various center-wall configurations.
  
* **`recentreIfNeeded`**: During the **Obstacle Challenge**, this function sets the servo steering angle to recentre the robot into its intended trajectory. It considers the last detected PixyCam color signature to decide which ultrasonic sensor to use for re-centering, to distinguish between obstacles and walls.

* **`correctionCooldown()`**: Controls the cooldown period for wall corrections. In the **Open Challenge**, it resets the `correctionState` flag after a fixed interval (`correctionCooldownMillis`, typically 1200 ms), enabling new corrections. In the **Obstacle Challenge**, the cooldown resets after every turn, ensuring that only one correction is made after each obstacle avoidance, to avoid misalignment caused by unnecessary adjustments.

For a more detailed analysis of the ultrasonic sensors integration for each challenge and the noise filtering function, refer to [**Ultrasonic Distance Sensing**](./08_ultrasonic_distance_sensing.md).

---

### Vision-Based Obstacle Evasion

VizDrive uses the **PixyCam 2.1** to detect and react to colored obstacles in its path.

* **`obstacleDetected()`**: Determines whether an obstacle is within close proximity, based on its position in the camera's frame. Returns `true` if an object is found within a predefined vertical threshold, `false` otherwise.
* **`handleEvasion()`**: Executes a blocking maneuver to steer the robot around an obstacle detected within close range. It identifies the largest obstacle within the ROI and adjusts the robot's steering based on the object's color signature:
    * **Signature 1 (Red)**: Triggers an evasion maneuver to the **right**.
    * **Signature 2 (Green)**: Triggers an evasion maneuver to the **left**.
The function tracks the X-coordinate of the obstacle, and only considers the evasion maneuver complete if the obstacle is detected out of the robot's central path, according to the color signature (e.g., red to the left side, or green to the right side).

**Debugging:**

* **`debugPixy()`**: Provides serial output of all detected blocks, including their coordinates and signatures. This function is typically commented out in optimized code to conserve processing power but was used during calibration and troubleshooting.

For a comprehensive explanation of PixyCam 2.1 integration and computer vision principles, consult [**Computer Vision Functions**](./07_pixycam_computer_vision.md).

---

### Color Detection

This module is responsible for identifying specific color patterns on both the track surface and side walls, crucial for autonomous navigation and the parking maneuver.

#### 1. Floor Detection (I2C TCS34725 Sensor)

This sensor, mounted underneath the robot, detects navigation cues on the track.

* **Blue**: Indicates a requirement for a **left** turn.
* **Orange**: Indicates a requirement for a **right** turn.
* **`detectFloorColor()`**: Reads raw RGB data from the sensor and normalizes these readings using the clear channel value (`c`) to compensate for ambient light variations. It then compares normalized values against predefined color thresholds to identify the floor color. Importantly, during the robot's **first lap**, this function defines the `direction` variable: **-1 for counter-clockwise** (if blue is detected) and **+1 for clockwise** (if orange is detected).
* **`handleColorAction()`**: Manages the robot's actions when a floor color is detected and no turn is currently in progress. It increments `lapTurnCount`, turns on an LED for visualization, and calculates the `turnTargetYaw` for a 90-degree turn based on the `direction`. It also sets `turningInProgress` to `true` to prevent re-triggering and increases `motorSpeed` to `250` for the turn. During the first lap turn, it includes a `safeDelay(500)` for initial adjustment.

#### 2. Wall Detection (Dual TCS3200 Sensors)

Two digital TCS3200 sensors, positioned on the robot's sides, are used primarily for detecting the parking trigger.

* **Magenta**: Detected after completing a specific number of laps (e.g., 4 laps), signaling the parking zone.
* **`detectWallMagenta()`**: Reads filtered RGB values from the appropriate side sensor (selected based on the current turning `direction`) and compares them against magenta-specific thresholds.

**Thresholds:**

* All color detection relies on **empirically tuned** RGB intensity thresholds (0-255) for accurate color identification under varying conditions.

For an in-depth explanation of color sensor operation, calibration, and detection logic, refer to [**Color Detection Functions**](./09_color_detection.md).

---

### Orientation Control

This module ensures the robot maintains a stable trajectory and executes precise turns using an MPU6050 gyroscope and a Proportional-Derivative (PD) control loop.

* The **MPU6050 gyroscope** provides angular velocity data (specifically from its Z-axis) which is integrated to track the robot's current yaw angle.
* **`updateOrientation(dt)`**: The time differential in this function is managed differently according to the challenge round. However, in general, it accumulates the robot's `yaw` angle by integrating the calibrated angular velocity (`gyroZ_deg`) over time (`dt`). And incorporates a `0.1` degree/second noise threshold.
* **`keepOrientation()`**: Implements the PD controller. It calculates an `error` between the `targetYaw` and the current `yaw`, then applies a proportional (`Kp`) and derivative (`Kd`) correction to the steering servo angle (`SERVO_STRAIGHT - (int)(Kp * error + Kd * derivative)`) to guide the robot back to its target heading.
* **`setTargetYaw(angle_deg)`**: Dynamically defines the desired yaw angle for the robot to maintain or achieve during maneuvers (e.g., setting a 90-degree turn target).
* **`getCurrentYaw()`**: Returns the current accumulated `yaw` angle of the robot.
* **`getAbsoluteYawError()`**: Calculates and returns the absolute difference between the `targetYaw` and the current `yaw`, used to determine if a turn has been completed.
* **`completedTurn()`**: Checks if a turn is `turningInProgress` and if the `getAbsoluteYawError()` is below the `TURN_THRESHOLD`. If so, it signifies turn completion, turns off an LED, resets `turningInProgress` and `correctionState`. In the Open Challenge, it reduces `motorSpeed` to `160` for subsequent color detection.
* **Gyroscope Calibration**: An initial calibration routine at setup compensates for any inherent bias or drift in the MPU6050's Z-axis gyroscope readings by averaging `samples` (250) readings to determine `gyroZ_offset`.

**Tuning Parameters:**

* `Kp = 1.8`: Proportional gain, influencing the response to the current heading error.
* `Kd = 1.2`: Derivative gain, influencing the response to the rate of change of the heading error, helping to dampen oscillations.
* `Ki = 0.0`: Integral gain, unused, but could be implemented to eliminate steady-state errors.

For a detailed exploration of MPU6050 integration, calibration, and the PD control algorithm (explaining the exclusion of the integral term), consult [**PID Control for the Gyroscope**](./06_pid_gyroscope_control.md).

---

## 3.3 Program Flow and State Management

The `main.ino` file orchestrates the robot's behavior, with the `setup()` function initializing the system and the `loop()` function continuously managing operational states and transitions.

### Initialization (`setup()`)

The `setup()` function executes once at program startup:

1.  **Serial Communication**: Initiates `Serial.begin(115200)` for debugging and monitoring (matching PixyCam's requirement).
2.  **Module Initialization**: Calls initialization for all hardware and software modules:
    * `initMovement()`: Configures motors, servo, and encoder.
    * `initOrientation()`: Initializes MPU6050 and performs gyroscope calibration.
    * `initVision()`: Prepares the PixyCam for operation.
    * `initUltrasonic()`: Sets up ultrasonic distance sensors.
    * `initColorSensors()`: Initializes both I2C and digital color sensors.
    * `initStateLogic()`: Establishes initial state variables, including `direction` and `targetYaw`.
3.  **Start Condition**: Enters a waiting loop (`waitForStartButton()`) until a physical button press signals the start of autonomous operations. A LED (`ledPin`) illuminates while waiting.
4.  **Timing Baseline**: Records `lastUpdateTime` using `millis()` to establish a timestamp for loop timing.

### Main Loop (`loop()`)

The `loop()` function runs continuously, performing core functionalities in a timed sequence:

1.  **Timing Control**: Ensures that main loop updates occur approximately every 30 milliseconds (roughly 33 Hz), calculating `dt` (delta time) for precise orientation updates, and updating `lastUpdateTime`.
2.  **Continuous Movement**: `driveForward(motorSpeed)` maintains the robot's forward progression.
3.  **Orientation Correction**:
    * `updateOrientation(dt)`: Regularly updates the robot's `yaw` angle.
    * `keepOrientation()`: Applies the PD control logic to adjust steering, maintaining the `targetYaw`.
    * `avoidWall(avoidance_amount)`: Realigns the robot relative to the walls using ultrasonic sensors, with `avoidance_amount` (8 degrees) specifying the correction.
    * `correctionCooldown()`: Manages the time between `avoidWall()` executions.
4.  **Obstacle Management**:
    * `if (obstacleDetected())`: Checks for obstacles using the PixyCam.
    * `handleEvasion()`: Executes the evasion maneuver if an obstacle is found.
5.  **Color-Based Action**:
    * `if (detectFloorColor())`: Checks for relevant color signals (floor lines).
    * `handleColorAction()`: Manages specific actions such as track turns, determined by the detected floor color.
6.  **Progress State & Shutdown**:
    * `if (turningInProgress)`: Checks the turning in progress flag. If there is a turn in progress, executes the `completedTurn()` function.
    * `completedTurn()`: Checks for the absolute yaw error; if it is below the `TURN_THRESHOLD`, it considers a turn completed.
    * `checkForShutdown()`: Monitors the `lapTurnCount` value; if the required number of laps are completed, the robot waits to arrive to the defined stopping area and halts all operations.

---

## 3.4 State Variables and Logic

The robot's dynamic operational state is managed through a set of global boolean and integer flags:

### State Variables

| Variable          | Type           | Purpose                                                                                                                                                                                                 |
| :---------------- | :------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `direction`       | `int`          | Defines the turning direction: -1 = counter-clockwise (left), +1 = clockwise (right). It's set during the first turn by `detectFloorColor()`.                                                              |
| `lapTurnCount`    | `unsigned int` | Counts the number of turns completed in the current lap. A full lap is defined as having 4 turns.                                                                                                |                                                   |
| `SystemShutdown`  | `bool`         | When set to `true`, this flag triggers `checkForShutdown()`, which forcefully stops the motors (`stopBrake()`) and enters an infinite loop, effectively halting all robot activity.                      |
| `turningInProgress` | `bool`         | Prevents re-triggering a turn while one is actively being executed. It is set to `true` upon floor color detection and turn initiation, and `false` when `getAbsoluteYawError()` is within `TURN_THRESHOLD`, signaling turn completion. |
| `correctionState` | `bool`         | Flag used to manage a cooldown or limit for ultrasonic corrections. It's set to `true` after an `avoidWall()` correction and reset to `false` by `correctionCooldown()` after a delay in the **Open Round**. It is reset after every completed turn in the **Obstacle Challenge**.                             |
| `lastCorrectionTime` | `unsigned long` | Stores the `millis()` timestamp of the last wall correction, used by `correctionCooldown()` to manage the cooldown period.                                                                         |
| `turnTargetYaw`   | `float`        | Stores the yaw angle the robot aims to achieve during a turn, calculated as `(currentYaw + 90.0 * -direction)`.                                                                                         |
| `targetYaw`       | `float`        | The desired yaw angle for the robot to maintain, actively corrected by the `keepOrientation()` PID loop.                                                                                                |
| `lastUpdateOrientationTime`       | `unsigned long`        | In the Obstacle Challenge Round, the `updateOrientation()` function does not require to be given the time differential, it stores and calculates it with the `lastUpdateOrientationTime` variable.        |

---

### Non-Blocking Delay (`void safeDelay(unsigned long ms)`)

* **Purpose**: Provides a mechanism for pausing execution for a specified duration (`ms`) without completely freezing the robot's critical sensor updates and control logic.
* **Parameters**:
    * `ms`: The duration of the delay in milliseconds.
* **Operation**: Unlike a standard `delay()`, `safeDelay()` continuously executes essential functions such as `updateOrientation()`, `detectFloorColor()`, and `handleColorAction()` within its loop. This ensures the robot remains responsive to environmental changes even during brief pauses in its main sequence. It still is a blocking delay; however, executes the necessary functions for the robot's adaptability.

---

## 3.5 Code Organization

The provided source code is organized into distinct modules (e.g., `main.ino`, `color_detection.cpp`, `movement.cpp`). While presented as a single integrated file for this documentation, in a larger development environment, these would typically be in separate `.cpp` and `.h` files within a `src/` directory. This separation improves code modularity, maintainability, and readability.

---

[Back to Main README.md Index](../README.md)
