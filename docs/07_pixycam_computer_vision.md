# 7. Vision and Distance Sensing

VizDrive utilizes the PixyCam 2.1 to detect and react to obstacles in real time.

---

## 7.1 PixyCam 2.1 for Vision-Based Obstacle Evasion

The **PixyCam 2.1** is an intelligent, compact vision sensor that provides rapid object recognition based on pre-trained color signatures. Its integrated processor significantly offloads computational burden from the main microcontroller, enabling real-time obstacle detection.

### Global Object and Configuration Parameters

```cpp
Pixy2 pixy; // PixyCam 2.1 object for communication and control

#define SIGNATURE_RED 2   // Defined signature ID for red obstacles
#define SIGNATURE_GREEN 1 // Defined signature ID for green obstacles

/* Configuration of the Region of Interest
   Coordinates of the Pixy resolution: width: 316, height: 208 */

// Y-coordinate threshold for an object to be considered "near" and start evasion.
// Any part of the object below this Y-coordinate will trigger detection.
const int ROI_Y_BOUND = 208 / 6;

const int IMAGE_WIDTH = 316;
// Define sections of the image for evasion completion check
const int LEFT_SECTION_END_X = IMAGE_WIDTH / 3;      // X-coordinate separating left third
const int RIGHT_SECTION_START_X = (2 * IMAGE_WIDTH) / 3; // X-coordinate separating right third
```

### Initialization (`void initVision()`)

  * **Purpose**: Establishes communication with and initializes the PixyCam 2.1 module.
  * **Operation**:
      * `pixy.init();`: Calls the initialization function from the `Pixy2` library. This function configures the SPI interface and prepares the camera for block detection.

### Obstacle Detection (`bool obstacleDetected()`)

  * **Purpose**: Determines if any detected colored object ("block") is currently present within the crucial lower region (below `ROI_Y_BOUND`).
  * **Operation**:
    1.  `pixy.ccc.getBlocks();`: Commands the PixyCam to process the current frame and retrieve a list of detected blocks (objects with assigned color signatures).
    2.  `if (pixy.ccc.numBlocks == 0) return false;`: If no blocks are reported by the PixyCam, no obstacles are present, and the function returns `false`.
    3.  **Iterative ROI Check**: The function then iterates through all detected `pixy.ccc.blocks`. For each block:
          * `if (pixy.ccc.blocks[i].m_y + (pixy.ccc.blocks[i].m_height / 2) > ROI_Y_BOUND)`: Checks if the bottom edge of the block extends beyond the `ROI_Y_BOUND`. If an object is found in this range, it is considered an obstacle, and the function immediately returns `true`.
    4.  If the loop completes without identifying any blocks in the crucial lower region, the function returns `false`.

### Obstacle Evasion Logic (`void handleEvasion()`)

  * **Purpose**: Manages the robot's steering to actively evade a detected obstacle, continuing until the path is clear.
  * **Operation**:
    ```cpp
    if (!obstacleDetected()) {
      return;
    }
  
    // --- Start of the blocking evasion loop ---
    // Once this point is reached, the car is committed to evading.
    Serial.println("--- EVASION PROTOCOL INITIATED ---");
  
    int activeSignature = 0; // Stores the signature of the obstacle being evaded (1 for red, 2 for green).
  
    // This loop will now run indefinitely until the exit condition is explicitly met.
    while (true) {
      pixy.ccc.getBlocks(); // Get the latest block data in every iteration.
      updateOrientation();
      if (detectFloorColor()) {
        handleColorAction();
        break;
      }
    ```

    1.  **Initial Obstacle Check**: `if (!obstacleDetected()) return;`: The function first verifies the presence of a nearby obstacle. If none is detected, the evasion protocol is not initiated.
    2.  **Continuous Evasion Loop**: Upon detecting an obstacle, the robot enters a `while(true)` loop, committing to the evasion process. This loop ensures the robot continues evasion maneuvers until an explicit exit condition is met.
          * `pixy.ccc.getBlocks();`: At the beginning of each loop iteration, updated block information is retrieved.
          * `updateOrientation();`: Ensures the robot's current heading is continuously monitored.
          * `if (detectFloorColor()) { handleColorAction(); break; }`: Allows for an early exit from the evasion loop if a floor color (turn signal) is detected, transitioning back to primary navigation.
    ```cpp
    int selectedIndex = -1;
    int largestArea = 0;
 
    // 2. Find the largest, most relevant obstacle currently in the "near" zone.
    for (int i = 0; i < pixy.ccc.numBlocks; i++) {
      if (pixy.ccc.blocks[i].m_y + (pixy.ccc.blocks[i].m_height / 2) > ROI_Y_BOUND) {
        int area = pixy.ccc.blocks[i].m_width * pixy.ccc.blocks[i].m_height;
        if (area > largestArea) {
          largestArea = area;
          selectedIndex = i;
        }
      }
    }
    ```
    3.  **Largest Relevant Obstacle Selection**:
          * The function iterates through all detected blocks to find the largest one (based on `width * height`) that is also within the `ROI_Y_BOUND`. This ensures the robot prioritizes reacting to the most prominent obstruction.
    4.  **Maneuver Based on Signature**: If a valid, relevant block is selected:
          * `int currentSignature = pixy.ccc.blocks[selectedIndex].m_signature;`: The color signature of the selected block is read.
          * `if (activeSignature == 0)`: This condition ensures that the evasion direction is determined only once at the beginning of the evasion sequence, based on the first detected obstacle's signature.
              * `if (activeSignature == SIGNATURE_RED) { setSteeringAngle(SERVO_RIGHT); lastSignature = SIGNATURE_RED; }`: If the obstacle is `SIGNATURE_RED`, the robot steers to the right.
              * `else if (activeSignature == SIGNATURE_GREEN) { setSteeringAngle(SERVO_LEFT); lastSignature = SIGNATURE_GREEN; }`: If the obstacle is `SIGNATURE_GREEN`, the robot steers to the left.
              * `else { setSteeringAngle(SERVO_STRAIGHT); lastSignature = 0; break; }`: If an unrecognized signature is encountered, the robot attempts to steer straight and exits the evasion loop.
    5.  **Evasion Completion Check**: This is the primary exit condition for the `while` loop. The robot breaks evasion only when the designated obstacle has been successfully navigated past and is located in a "safe" horizontal section of the image.
          * `if (activeSignature == SIGNATURE_RED && currentX < LEFT_SECTION_END_X)`: If a red obstacle was being evaded, and its `currentX` coordinate is now in the leftmost third of the image, evasion is considered complete.
              * A `safeDelay(500)` is introduced, followed by `setSteeringAngle(SERVO_LEFT)` with `safeDelay()`, depending on yaw error, to ensure a full pivot past the obstacle before breaking the loop.
          * `else if (activeSignature == SIGNATURE_GREEN && currentX > RIGHT_SECTION_START_X)`: Similarly, if a green obstacle was being evaded and its `currentX` coordinate is now in the rightmost third of the image, evasion is complete.
              * A `safeDelay(600)` is introduced, followed by `setSteeringAngle(SERVO_RIGHT)` with `safeDelay()` to ensure a full pivot.
    6.  **Loss of Sight Handling**: If `selectedIndex` remains `-1` (meaning no block is currently detected in the "near" zone), the robot continues to execute the last commanded steering action. This prevents premature termination of the evasion sequence if the camera momentarily loses sight of the obstacle.
    7.  **Evasion Completion**: Once the `while` loop is broken (either by successful evasion or floor color detection), the evasion protocol is considered complete. The robot's steering will typically revert to `SERVO_STRAIGHT` in the subsequent iteration of the main program loop.


  <img src="../assets/gif_animations/Evasion.gif" width="400" alt="Evasion">

### Debugging Function (`void debugPixy()`)

  * **Purpose**: Provides detailed real-time information about blocks detected by the PixyCam to the Serial Monitor, aiding in calibration and troubleshooting.
  * **Operation**: It iterates through `pixy.ccc.numBlocks` and prints the index, X/Y coordinates, and color signature of each block. It also explicitly indicates whether each block falls within the defined ROI (`LEFT_BOUND` and `RIGHT_BOUND`). This function is typically commented out in the main code for performance optimization but is invaluable during development.

-----

## 7.2 PixyMon Configuration and Parameters

The PixyCam 2.1 requires initial setup and color signature training via the **PixyMon** software (compatible with Windows, macOS, and Linux). This graphical interface enables precise calibration of color signatures and adjustment of various camera parameters, which are then saved directly to the PixyCam's firmware.

Here are the key configurations applied in PixyMon for obstacle detection:

| Configuration Area | Parameter / Setting          | Value / Description                                                                                                                                                                                                           | Image |
| :----------------- | :--------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :---------------------------: | 
| **Program** | `File > Set program`         | **`ccc_program`**: This program (Color Connected Components) is selected as it is specifically designed for detecting and tracking color-coded objects. | <img src="../assets/software_photos/Pixymonprogram.png" width="1000">     |
| **Signatures** | `Action > Set Signature`     | **`Signature 1 (Green)`** and **`Signature 2 (Red)`**: These specific signature IDs are assigned to the green and red obstacles on the competition field. Each signature is 'taught' by drawing a bounding box around the target object in the live video feed. | <img src="../assets/software_photos/Pixymonsignature.png" width="1000">     |
|                    | `Configure > Signature N`    | **Hue, Saturation, Value (HSV) Ranges**: After teaching, these ranges are meticulously fine-tuned for each signature. The goal is to optimize detection reliability under varying ambient light conditions and to minimize false positives from background colors. | <img src="../assets/software_photos/Pixymonsignatures2.png" width="1000">     |
| **Camera** | `Configure > Camera`         | **Brightness**: Adjusted to an optimal level to ensure good image exposure without over-saturation or underexposure.                                                                                                   | <img src="../assets/software_photos/Pixymoncamera.png" width="1000">     |
|                    | `Configure > Camera`         | **Auto White Balance**: **Enabled (`Auto`)** to allow the camera to automatically adapt to different lighting temperatures on the field, ensuring consistent color interpretation.                                          | <img src="../assets/software_photos/Pixymoncamera.png" width="1000">     |
|                    | `Configure > Camera`         | **Auto Exposure**: **Enabled (`Auto`)** for automatic adjustment of exposure settings. This is crucial for adapting to dynamically changing light intensities throughout the mission.                                     | <img src="../assets/software_photos/Pixymoncamera.png" width="1000">     |
|                    | `Configure > Camera`         | **LED Mode**: Set to `Auto` or `On` as needed to ensure proper illumination of detected objects for consistent signature recognition.                                                                                         | <img src="../assets/software_photos/Pixymoncamera.png" width="1000">     |
| **Interface** | `Configure > Interface`      | **Data Out Port**: Set to **`SPI`** to enable high-speed serial communication with the Arduino Mega.                                                                                                                      | <img src="../assets/software_photos/Pixymoninterface.png" width="1000">     |
| **Expert** | `Configure > Expert`         | **Max Blocks**: Set to a low value (e.g., `2`) to limit the number of reported blocks, focusing on the most dominant obstacles.                                                                           | <img src="../assets/software_photos/Pixymonexpert1.png" width="1000">     |
|                    | **Min Block Size (Width/Height)** | Configured to filter out very small (and likely distant or noisy) detections, ensuring only relevant obstacles are reported.                                                                                           | <img src="../assets/software_photos/Pixymonexpert2.png" width="1000">     |
|                    | **Merge Blocks** | **Enabled**: This setting is activated to instruct the PixyCam to merge adjacent detections of the same color signature into a single, larger block, improving robustness for objects that might be detected as fragmented. | <img src="../assets/software_photos/Pixymonexpert3.png" width="1000">     |

---

![Pixymon Software](../assets/software_photos/Pixymon.png)

[Back to Main README.md Index](./../README.md)
