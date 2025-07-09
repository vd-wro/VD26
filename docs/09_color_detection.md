# 9. Color Sensors for Circuit Perception

VizDrive's autonomous navigation relies heavily on its ability to perceive crucial color cues from the competition circuit. This section provides a detailed explanation of the various color sensors implemented, their specific functions, communication protocols, and the associated data processing techniques.

---

## 9.1 Floor Color Sensor (I2C - Adafruit TCS34725)

A dedicated TCS34725 color sensor is mounted on the underside of the robot. Its primary function is to detect colored lines on the circuit mat, which serve as strategic indicators for turns.

### Global Object

```cpp
static Adafruit_TCS34725 floorTcs(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_1X); // I2C color sensor object
```

This object is initialized with an integration time of 2.4 milliseconds and a gain of 1X, parameters that control the sensor's light exposure and sensitivity.

### Communication

The TCS34725 sensor communicates with the Arduino Mega via the **I2C (Inter-Integrated Circuit)** bus. This two-wire serial interface (SDA and SCL) allows for efficient data exchange.

### Color Thresholds for Floor Detection

These empirically determined thresholds are crucial for identifying specific colors (Blue and Orange) after the raw sensor data has been normalized. They represent the minimum required intensity for a color channel or the maximum allowed intensity for others.

```cpp
// For BLUE color detection (expected intense blue channel, and mild red and green colors)
const int BLUE_RED_THRESHOLD = 80;
const int BLUE_GREEN_THRESHOLD = 85;
const int BLUE_BLUE_THRESHOLD = 70;

// For ORANGE color detection (expected intense red and green colors, and mildly intense blue colors)
const int ORANGE_RED_THRESHOLD = 110;
const int ORANGE_GREEN_THRESHOLD = 100;
const int ORANGE_BLUE_THRESHOLD = 70;
```

### Initialization (`void initColorSensors()`)

* **Purpose**: Initializes the floor-facing I2C color sensor and calls the initialization for the digital wall color sensors.
* **Operation**:
  * `floorTcs.begin();`: This command initiates the I2C communication with the TCS34725 sensor and configures it for operation.

### `bool detectFloorColor()`

* **Purpose**: Detects specific turn-signal colors (Blue or Orange) on the track based on the robot's expected turning direction.
* **Operation**:
    1. **Raw Data Acquisition**: `uint16_t r,g,b,c; floorTcs.getRawData(&r,&g,&b,&c);` retrieves the raw red, green, blue, and clear light intensity values from the TCS34725 sensor.
    2. **Sensor Validation**: `if (!c) return false;`: If the clear light reading (`c`) is zero, it indicates a sensor error or insufficient ambient light, and the function returns `false`.
    3. **Color Normalization (TCS34725)**:
          * `int rn=r*255/c, gn=g*255/c, bn=b*255/c;`: The raw R, G, B values are normalized to a 0-255 scale by dividing each channel by the clear light value (`c`). This normalization is essential to compensate for variations in ambient light conditions, ensuring that color detection remains consistent regardless of light intensity. For example, a red object will always have a high "redness" ratio relative to its total brightness, even if the overall light is dim.
    4. **Color Recognition based on `direction`**:
          * If the global `direction` is `-1` (indicating an expected left turn), the function checks for **Blue** by comparing `bn` against `BLUE_BLUE_THRESHOLD` (high blue) and `rn`, `gn` against `BLUE_RED_THRESHOLD`, `BLUE_GREEN_THRESHOLD` (low red and green).
          * If the global `direction` is `+1` (indicating an expected right turn), the function checks for **Orange** by comparing `rn`, `gn` against `ORANGE_RED_THRESHOLD`, `ORANGE_GREEN_THRESHOLD` (high red and green) and `bn` against `ORANGE_BLUE_THRESHOLD` (low blue).
    5. Returns `true` if the expected color for the current turn `direction` is detected, otherwise `false`.

---

## 9.2 Wall Color Sensors (Digital - TCS3200)

Two digital TCS3200 color sensors are strategically mounted on the sides of the robot. These sensors are primarily used for detecting the magenta "parking signal" on walls during the final phase of the mission.

### Hardware Definitions

The following pins are used to control and read data from the left and right TCS3200 sensors:

```cpp
// ------------- Wall sensors (digital TCS3200) -------------
#define S0_L 10 // Left sensor frequency scaling pin S0
#define S1_L 12 // Left sensor frequency scaling pin S1
#define S2_L 16 // Left sensor color filter pin S2
#define S3_L 18 // Left sensor color filter pin S3
#define OUT_L 48 // Left sensor output frequency pin
#define S0_R 51 // Right sensor frequency scaling pin S0
#define S1_R 49 // Right sensor frequency scaling pin S1
#define S2_R 50 // Right sensor color filter pin S2
#define S3_R 52 // Right sensor color filter pin S3
#define OUT_R 53 // Right sensor output frequency pin
```

### Global Structure and Color Thresholds

```cpp
struct RGB { // A simple struct to hold normalized RGB values
  int red;
  int green;
  int blue;
}

// For MAGENTA color detection (expected intense red and blue colors, and mildly intense green colors)
const int MAGENTA_RED_THRESHOLD = 150;
const int MAGENTA_GREEN_THRESHOLD = 120; // Lower green intensity expected for magenta
const int MAGENTA_BLUE_THRESHOLD = 150;
```

### Initialization (`void initWallColorSensors()`)

* **Purpose**: Configures the digital I/O pins for the left and right TCS3200 color sensors.
* **Operation**:
    1. Sets all `S0_L`, `S1_L`, `S2_L`, `S3_L`, `OUT_L` pins for the left sensor and `S0_R`, `S1_R`, `S2_R`, `S3_R`, `OUT_R` pins for the right sensor as `OUTPUT` or `INPUT` as appropriate.
    2. `digitalWrite(S0_L, HIGH); digitalWrite(S1_L, HIGH);`: The `S0` and `S1` pins control the output frequency scaling of the TCS3200. Setting both to `HIGH` typically configures the sensor to output at 100% frequency scale, providing maximum resolution for readings.

### `RGB readTCS3200(int s2, int s3, int outPin)`

* **Purpose**: Reads the raw R, G, B color components from a single TCS3200 sensor by measuring its output frequency for each color filter.
* **Parameters**:
  * `s2`, `s3`: The pins used to select the color filter (Red, Green, or Blue) on the TCS3200.
  * `outPin`: The output frequency pin of the TCS3200.
* **Operation**:
    1. **Filter Selection and Frequency Measurement**:
          * **Red**: `digitalWrite(s2, LOW); digitalWrite(s3, LOW);` (selects red filter). `delay(5);` `int redRaw = pulseIn(outPin, LOW);` (measures the pulse width for red light).
          * **Green**: `digitalWrite(s2, HIGH); digitalWrite(s3, HIGH);` (selects green filter). `delay(5);` `int greenRaw = pulseIn(outPin, LOW);`
          * **Blue**: `digitalWrite(s2, LOW); digitalWrite(s3, HIGH);` (selects blue filter). `delay(5);` `int blueRaw = pulseIn(outPin, LOW);`
          * **Note on `pulseIn()`**: The `pulseIn()` function returns the duration of a pulse in microseconds. For the TCS3200, this duration is *inversely proportional* to the light intensity for the selected color. Higher light intensity results in higher output frequency, which means a *shorter* pulse duration.
    2. **Normalization and Inversion (TCS3200)**:
          * `int maxRaw = max(max(redRaw, greenRaw), blueRaw); if (maxRaw == 0) maxRaw = 1;`: Finds the maximum raw pulse width among the three colors. This maximum is used as a reference for normalization. A check for `maxRaw == 0` prevents division by zero.
          * `rgb.red = map(constrain(redRaw, 0, maxRaw), 0, maxRaw, 255, 0);`:
              * `constrain(redRaw, 0, maxRaw)`: Ensures the raw reading is within a valid range relative to `maxRaw`.
              * `map(..., 0, maxRaw, 255, 0)`: This is the crucial **inversion and scaling** step. Since `pulseIn` returns a duration (where a *smaller* value means *more* light), the `map` function is used to convert this inverse relationship into a standard intensity scale (0-255). A raw reading of `0` (maximum light/frequency) is mapped to `255` (maximum intensity), and `maxRaw` (minimum light/frequency) is mapped to `0` (minimum intensity). This process is repeated for the green and blue channels.
    3. Returns an `RGB` struct containing the normalized and inverted R, G, B values.

---

## 9.3 Turn Maneuver Logic (`void handleColorAction()`)

This function is critical for the robot's autonomous navigation, orchestrating precise turning maneuvers upon detecting a color cue on the circuit mat. Its implementation varies significantly between the **Obstacle Challenge** and **Open Challenge**.

### 9.3.1 For Obstacle Challenge

In the **Obstacle Challenge**, `handleColorAction()` executes a sequence of actions, integrating proximity sensing and precise realignments with walls. This detailed logic is crucial for navigating tight spaces and ensuring accurate positioning after turns.

  * **Purpose**: To execute a comprehensive turning and realignment sequence upon detecting a floor color, adapting to immediate wall proximity.
  * **Operation**:
    1.  **Detection and Pre-Conditions**:

          * `if (detectFloorColor() && !turningInProgress && turnCooldown())`: The function is activated by a floor color detection, provided no turn is currently in progress (`!turningInProgress`), and a cooldown period has elapsed since the last turn (`turnCooldown()`). This prevents repetitive re-detections.
          * `lapTurnCount++;`: Increments a counter tracking the number of turns completed in the current lap.
          * `turningInProgress = true;`: Sets a flag to denote that a turning maneuver is active, preventing redundant activations.
          * `digitalWrite(ledPin, HIGH);`: Activates an onboard LED for visual indication of the turning state.

    2.  **Frontal Wall Approach**:

          * `setSteeringAngle(SERVO_STRAIGHT);`: The robot's steering is first set to a straight-ahead position.
          * `int df = sonarFront.ping_cm(); safeDelay(10);`: An initial distance reading is acquired from the front-facing ultrasonic sensor (`sonarFront`).
          * `while (df > 10 || df == 0)`: The robot proceeds forward as long as the front distance (`df`) is greater than 10 cm.
              * `df = getDistance(sonarFront);`: The front distance is continuously updated.
              * `keepOrientation();`: The robot's yaw orientation is actively maintained via its control loop during this forward movement phase.
              * `safeDelay(10);`: A brief, non-blocking delay allows for orientation updates and control loop iterations.
          * `stopBrake();`: Once the robot approaches approximately 10 cm from the frontal wall, its motors are abruptly stopped.

    3.  **Lateral Distance Measurement and Yaw Target Calculation**:

          * `NewPing sonar = (direction < 0) ? sonarRight : sonarLeft;`: The appropriate lateral ultrasonic sensor (`sonarRight` for left turns, `sonarLeft` for right turns) is dynamically selected based on the global `direction` variable.
          * `int distance = getDistance(sonar);`: The distance to the corresponding lateral wall is obtained.
          * `turnTargetYaw = (turnTargetYaw + (turnAngle * -direction)); setTargetYaw(turnTargetYaw);`: The new target yaw for the turn (typically $\\pm 90^\\circ$) is computed and set based on the `turnAngle` and `direction`.

    4.  **Adaptive Turning Maneuver**: The turning strategy adapts based on the measured lateral wall distance:

          * **Scenario A: Close Lateral Wall (`if (distance <= 35 && distance != 0)`)**:
              * `driveBackward(motorSpeed); safeDelay(1500); stopBrake(); safeDelay(500);`: The robot executes a backward movement for 1500ms, followed by a brief stop, to position itself for the turn.
              * `driveForward(200);`: Initiates a forward movement to commence the turning arc.
              * `while(turningInProgress) { keepOrientation(); completedTurn(); safeDelay(5); }`: The robot continues the forward turn (`keepOrientation()`) and checks for turn completion (`completedTurn()`).
              * `driveBackward(250); safeDelay(2000);`: Upon turn completion, the robot reverses for 2000ms to intentionally collide with the new rear wall, ensuring an accurate yaw realignment.
              * `yaw = turnTargetYaw;`: The global `yaw` variable is reset to `turnTargetYaw` to correct any accumulated drift.
              * `driveForward(motorSpeed); safeDelay(250);`: The robot resumes forward motion.
          * **Scenario B: Far Lateral Wall (`else`)**:
              * `int angle = (direction < 0) ? SERVO_RIGHT+10 : SERVO_LEFT-10;`: The steering servo angle is adjusted to a sharper value, compensating for the greater distance to the wall.
              * `setSteeringAngle(SERVO_STRAIGHT); safeDelay(500); stopBrake(); safeDelay(250);`: The robot briefly straightens its steering and reverses.
              * `steeringServo.write(angle); safeDelay(150);`: The sharper steering angle is set, followed by a brief delay.
              * `driveBackward(250);`: Initiates a backward movement with the adjusted steering, allowing the robot to correct its orientation.
              * `while(turningInProgress) { completedTurn(); safeDelay(5); }`: The robot continues reversing until the turn is complete, with continuous checks for completion.
              * `setSteeringAngle(SERVO_STRAIGHT); driveBackward(250); safeDelay(1500);`: After the turn, the robot straightens and reverses to collide with the wall, re-aligning its yaw.
              * `yaw = turnTargetYaw;`: The global `yaw` is reset.
              * `driveForward(motorSpeed); safeDelay(250);`: Resumes forward motion.

    5.  **Function Exit**: In both turning scenarios, the function completes its execution, returning control to the main loop.

### 9.3.2 For Open Challenge

In the **Open Challenge**, `handleColorAction()` is streamlined for direct and efficient turning maneuvers. This design reflects the typically more open track layout, where precise wall interactions for realignment are not required.

  * **Purpose**: To detect a floor line and execute a direct 90-degree turn, without incorporating wall-based realignments, because they are incorporated throughout the trajectory.
  * **Operation**:
    1.  **Detection and Pre-Conditions**:
          * `if (detectFloorColor() && !turningInProgress)`: The function activates upon detecting a floor color, provided no turning maneuver is already in progress (`!turningInProgress`).
          * `if (lapTurnCount == 0)`: This conditional block addresses a specific initial turn behavior:
              * `lapTurnCount++;`: Increments the turn counter.
              * `digitalWrite(ledPin, HIGH);`: Illuminates the LED for state indication.
              * `delay(500);`: A blocking delay of 500 milliseconds is introduced. This may serve to ensure the robot fully crosses the detected line or to provide a brief stabilization period before initiating the rotation.
          * `turnTargetYaw = (turnTargetYaw + (90.0 * -direction)); setTargetYaw(turnTargetYaw);`: The new target yaw for a ±90° turn is calculated and set, based on the global `direction`.
          * `turningInProgress = true;`: Sets a flag indicating that a turn is now in progress, preventing immediate re-triggering by the color sensor.
          * `motorSpeed = 250;`: The motor speed is explicitly increased to `250` during the turn, for faster execution.
    2.  **Subsequent Turns**:
          * If `lapTurnCount` is not zero (i.e., it's not the very first turn), the specific `delay(500)` for the first turn is bypassed. This delay is planned for the robot to be adaptable to any starting scenario. Then, the subsequent turn logic is directly applied:
              * `lapTurnCount++;`: Increments the turn counter.
              * `digitalWrite(ledPin, HIGH);`: Illuminates the LED.
              * `turnTargetYaw = (turnTargetYaw + (90.0 * -direction)); setTargetYaw(turnTargetYaw);`: Calculates and sets the new yaw target.
              * `turningInProgress = true;`: Sets the turning in progress flag.
              * `motorSpeed = 250;`: Increases motor speed for the turn.
    3.  **Function Exit**: If the `if` condition for `detectFloorColor()` or `!turningInProgress` is not met, the function simply returns, awaiting the appropriate conditions for turn initiation.

### 9.3.3 Comparative Analysis of `handleColorAction()` Implementations

| Feature                | Obstacle Challenge Implementation                 | Open Challenge Implementation                 |
| :--------------------- | :------------------------------------------------ | :-------------------------------------------- |
| **Maneuver Complexity** | 
Implements a complex sequence involving **frontal wall proximity detection**, **dynamic selection of lateral ultrasonic sensors**, an **adaptive turning algorithm** based on wall distance, and a **controlled backward collision for precise yaw realignment**. | 
Employs a **direct, simplified 90-degree turn**. It lacks the multi-stage sensing, adaptive turning, or wall-collision-based realignment found in the Obstacle Challenge version. |
| **Sensor Utilization** | 
Uses both the **front-facing (`sonarFront`)** and **side-facing (`sonarRight`, `sonarLeft`) ultrasonic sensors** to guide turn initiation and and execution. | 
Relies primarily on the floor color sensor for turn initiation. Ultrasonic sensors are not integrated into the `handleColorAction()` logic for turn execution. |
| **Turn Trigger Filtering** | Includes a `turnCooldown()` mechanism alongside `!turningInProgress` to prevent rapid, unintended re-detections of the color line, which could occur in dynamic evasion maneuvers. | Utilizes only `!turningInProgress`, a cooldown is not necessary. |
| **Initial Turn Behavior** | All turns follow a consistent realignment sequence using the rear wall. | Uses a `if (lapTurnCount == 0)` condition for the very first turn, introducing a `delay(500)` that is omitted in subsequent turns, this is for adaptability to different center configurations. |
| **Yaw Realignment** | Includes a backward movement leading to a collision with a wall, followed by `yaw = turnTargetYaw;` to correct any accumulated orientation error. This "hard reset" of yaw is crucial when avoiding obstacles, as the MPU-6050 offset is greater. | Does not incorporate any wall collision or `yaw` reset within this function. Yaw management is implicitly handled by the broader `setTargetYaw` and `keepOrientation` mechanisms outside `handleColorAction()`. |
| **Motor Speed Adjustment** | Motor speed for turning is not explicitly modified within the function; it primarily focuses on steering and timed movements. | Explicitly sets `motorSpeed = 250;` during turns, to accelerate during rotation for faster maneuvers. |


---

## 9.4 Enhancing Detection with a Long-Distance Lens

Color sensors inherently detect light from a relatively wide field of view (FOV), which can be problematic when precise, long-distance color detection is required. To overcome this, particularly for identifying the magenta parking signal on walls from a greater distance, we integrated a long-distance lens with our TCS3200 sensors. This lens narrows the sensor's FOV, allowing it to focus on a small, specific area.

The TCS3200 features a 4x4 array of 16 photodiodes, each approximately 120 µm x 120 µm. This results in a tiny effective sensor size of roughly 0.48 mm x 0.48 mm, or a diagonal of approximately 0.68 mm.

### Field of View (FOV) Calculation

To quantify the effect of the lens, we calculated the approximate Field of View (FOV) using the following formula:

$FOV = 2 \\cdot \\arctan\\left(\\frac{d}{2f}\\right)$

Where:

  * $d$ = sensor diagonal ($0.68 \\text{ mm}$)
  * $f$ = focal length of the lens ($25 \\text{ mm}$)

Substituting our values:

$FOV = 2 \\cdot \\arctan\\left(\\frac{0.68 \\text{ mm}}{2 \\cdot 25 \\text{ mm}}\\right) = 2 \\cdot \\arctan(0.0136) \\approx 2 \\cdot 0.78^\\circ \\approx 1.56^\\circ$

This calculation demonstrates that with a 25 mm lens, the TCS3200 perceives a very narrow cone of vision, approximately 1.56 degrees wide. This narrow FOV enables the sensor to detect colors from a small, highly focused point, which is ideal for pinpoint detection at longer ranges.

### Spot Size at Distance

To further understand the practical implication of this narrow FOV, we calculated the approximate spot size that the sensor "sees" at a typical detection distance of 50 cm (500 mm):

$\\text{Spot Diameter} \\approx 2 \\cdot \\text{Distance} \\cdot \\tan\\left(\\frac{\\text{FOV}}{2}\\right)$

At a distance of 50 cm (500 mm), with a $\\text{FOV}$ of $1.56^\\circ$:

$\\text{Spot Diameter} \\approx 2 \\cdot 500 \\text{ mm} \\cdot \\tan\\left(0.78^\\circ\\right) \\approx 13.6 \\text{ mm}$

Therefore, at a distance of 50 cm, the TCS3200, equipped with the 25 mm lens, effectively "sees" a circular spot approximately 13.6 mm in diameter. This is important for detecting the magenta parking signal without interference from surrounding colors.

---

[Back to Main README.md Index](https://www.google.com/search?q=../README.md)
