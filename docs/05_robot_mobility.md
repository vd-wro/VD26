
# 5. Robot Mobility

This section provides an in-depth analysis of VizDrive's mobility system, covering mechanical design considerations, motor and steering configurations, and the software module responsible for locomotion.

---

## 5.1 Software Mobility Module

The `movement` module contains the core functions that govern VizDrive's locomotion, abstracting motor and steering control.

### Hardware Definitions

The following preprocessor definitions specify the Arduino pins connected to the motor driver, servo, and encoder:

```cpp
#define MOTOR_INA_PWM 4 // PWM pin for motor INA (DC Motor Driver)
#define MOTOR_INB_PWM 5 // PWM pin for motor INB (DC Motor Driver)
#define SERVO_PIN 6     // Pin for the steering servo motor
#define ENCODER_PIN 2   // Digital pin for the quadrature encoder interrupt
```

### Global Variables and Constants

* **`motorSpeed`**: `const int motorSpeed = 150;` (Defined in `main.ino`) - The default PWM value (0-255) for consistent motor speed during standard operations.
* **`steeringServo`**: `Servo steeringServo;` - The object representing the steering servo motor.
* **`encoderPulseCount`**: `volatile unsigned long encoderPulseCount = 0;` - A volatile counter that increments with each pulse from the encoder wheel, tracking distance traveled.

### Steering Angle Constants

These constants define the safe operational range for the steering servo:

```cpp
const int SERVO_STRAIGHT = 85; // Servo angle for straight-ahead motion
const int SERVO_MAX = 35;      // Maximum angular deflection from SERVO_STRAIGHT (maximum steering angle is 45 degrees)
const int SERVO_LEFT = SERVO_STRAIGHT - SERVO_MAX; // Minimum servo angle for extreme left turn (e.g., 50 degrees)
const int SERVO_RIGHT = SERVO_STRAIGHT + SERVO_MAX; // Maximum servo angle for extreme right turn (e.g., 120 degrees)
```

### Functions

#### `void driveForward(int speed)`

* **Purpose**: Engages the rear DC motors for forward propulsion.
* **Parameters**:
  * `speed`: An integer PWM value (0-255) that directly controls the motor's power and speed.
* **Operation**:
  * `analogWrite(MOTOR_INA_PWM, speed);`: Applies the specified PWM signal to control motor power.
  * `digitalWrite(MOTOR_INB_PWM, LOW);`: Sets the direction pin to `LOW` for forward movement.

#### `void driveBackward(int speed)`

* **Purpose**: Activates the rear DC motors to move the robot in reverse.
* **Parameters**:
  * `speed`: An integer PWM value (0-255) for backward motor power.
* **Operation**:
  * `analogWrite(MOTOR_INA_PWM, LOW);`: Sets the INA pin to `LOW`.
  * `digitalWrite(MOTOR_INB_PWM, speed);`: Applies the PWM signal to the INB pin, establishing the backward direction. This function is specifically utilized for controlled reverse maneuvers, such as during the parking sequence.

#### `void stopMotors()`

* **Purpose**: Deactivates the DC motors, allowing the robot to decelerate naturally.
* **Operation**:
  * `digitalWrite(MOTOR_INA_PWM, LOW);`
  * `digitalWrite(MOTOR_INB_PWM, LOW);`
        This method provides a gentle stop, reducing mechanical stress compared to an abrupt brake.

#### `void stopBrake()`

* **Purpose**: Implements an immediate and forceful braking action for the DC motors.
* **Operation**:
  * `digitalWrite(MOTOR_INA_PWM, HIGH);`
  * `digitalWrite(MOTOR_INB_PWM, HIGH);`
        By setting both motor input pins to `HIGH`, a short-circuit braking effect is achieved, resulting in a rapid stop. This is typically used for emergency halts or final system shutdown.

#### `void setSteeringAngle(int angle)`

* **Purpose**: Controls the angular position of the front steering wheels using the attached servo motor.
* **Parameters**:
  * `angle`: An integer representing the desired servo position in degrees (0-180).
* **Operation**:
  * `steeringServo.write(angle);`: Sends the `angle` command to the servo using the `Servo.h` library.
  * `angle = constrain(angle, SERVO_LEFT, SERVO_RIGHT);`: The `angle` value is constrained to remain within `SERVO_LEFT` and `SERVO_RIGHT` boundaries.
* **Safety**: It is important to constrain the angle into the maximum boundaries. This prevents over-rotation and physical interference with the chassis.

#### `void exitParking()`

* **Purpose**: Exits the parking zone by steering away from nearby walls based on real-time distance readings from ultrasonic sensors.
* **Operation**:
  1. **Distance Measurement**:
     ```cpp
     int distanceR = getDistance(sonarRight);
     int distanceL = getDistance(sonarLeft);
     ```
     The function begins by measuring the distance to the nearest  wall using the right and left ultrasonic sensors. These values are filtered by the `getDistance()` function to ensure stability and reduce noise.
     
  3. **Decision Logic Based on Proximity**:
     The robot determines which side is closer to a wall and selects an appropriate evasive maneuver:
     
     * **Wall on the Right**:

       ```cpp
       if (distanceR < 30 && distanceR != 0) {
           steeringServo.write(SERVO_STRAIGHT - 40);
           driveForward(220);
           safeDelay(1000);
       }
       ```
       If the right wall is within 30 cm and the reading is valid (not zero), the robot steers sharply to the **left** (by subtracting 40 from the straight angle) and drives forward at a moderate speed. The movement is sustained for 1000 milliseconds using a non-blocking `safeDelay()` to allow for orientation updates.

     * **Wall on the Left**:

       ```cpp
       else if (distanceL < 30 && distanceL != 0) {
           steeringServo.write(SERVO_STRAIGHT + 40);
           driveForward(220);
           safeDelay(1000);
       }
       ```
       
     Similarly, if the left wall is within 30 cm, the robot steers sharply to the **right** (adding 40 to the straight angle) and drives forward with the same parameters.

     * If neither wall is detected within the 30 cm threshold, the robot assumes it is centered and skips the exit parking maneuver entirely.

#### `void parkingManeuver()`

* **Purpose**: Executes a predefined parking maneuver, adapting to the robot's current turning `direction`.

* **Operation**:

    1. **Defining Math and Control Variables**

        ```cpp
        int distance = getDistance(sonarLeft); // The distance to the wall
        // Pythagorean theorem
        int a_cm = distance - 28; // a_cm is negative if the bot is too near to the wall
        int b_cm = 75; // Longitudinal distance
        int c_cm; // Diagonal distance
        int angle; // Angle of turn
        int turnDirection; // Used to know the direction of the turn; if a_cm is 0, this variable is 0 (no turn is required)
        ```
        Variables are changed according to the parking maneuver to be executed, which depends on the `direction`.

        The diagonal distance `c_cm` and the `angle` required to get near the parking lot is calculated using geometric concepts and trigonometry. 

    2. **Defining Turn Direction**

        ```cpp
        if  (a_cm > 0) { // Close to wall
          c_cm = sqrt(pow(a_cm, 2) + pow(b_cm, 2)); // Calculates the amount of pulses for diagonal movement
          turnDirection = 1;
        }
        else if (a_cm < 0){ // Far from wall
          c_cm = sqrt(pow(a_cm, 2) + pow(b_cm, 2)); // Calculates the amount of pulses for diagonal movement
          turnDirection = -1;
        } else { 
          c_cm = b_cm; // No amount of diagonal movement
          turnDirection = 0;
        }
        ```
        The `turnDirection` variable defines the orientation of the turn. If the robot is near the wall, it will proceed to turn outwards; conversely, if it's far from the parking, it will turn inwards. Moreover, there is a possibility `a_cm = 0`. For this case, no turn is required, indicating that `turnDirection` will be null.

    3. **Calculating Turn**

        ```cpp
        if (turnDirection != 0) { // Diagonal movement is required
          float tangent = ((float)abs(a_cm)/b_cm); // Calculate tangent
          angle = (int)(atan(tangent) * (180.0 / M_PI)); // Calculate required turn angle

          turnTargetYaw = (turnTargetYaw + (angle*direction*turnDirection)); // Adjustes target yaw
          setTargetYaw(turnTargetYaw); 
        } else { // No angle was required
          Serial.print(" - No angle required - ");
        }
        ```
        The required angle is calculated and applied to the target yaw.

    4. **Movement Execution**:

        ```cpp
        encoderDelayOrientation(c_cm); // Waits until the defined distance is complete
        stopBrake();  // Stops the motors

        if (turnDirection != 0) { // Resets angles when distance is complete
          turnTargetYaw = (turnTargetYaw - (angle*direction*turnDirection));  // Resets the target yaw
          setTargetYaw(turnTargetYaw); // Applies the target yaw
        }
        ```

        The movement is executed, and the target yaw is reset, positioning the robot past the parking lot, aligned parallel to the outer wall.

    6. **Parking Sequence**: The robot moves backwards, the steering servo is turned to the direction of the parking, it reverses and steers to the opposite direction, calculating the amount of steps using `encoderDelay()`, completing the parking maneuver.

#### `void encoderISR()`

* **Purpose**: This is an Interrupt Service Routine (ISR) designed to detect pulses from the quadrature encoder attached to the drivetrain.
* **Operation**:
  * `encoderPulseCount++;`: Increments the `encoderPulseCount` variable every time a rising edge is detected on the `ENCODER_PIN`. This ISR is attached to the hardware interrupt for `ENCODER_PIN`, ensuring highly accurate pulse counting for distance measurement.

---

[Back to Main README.md Index](./../README.md)
