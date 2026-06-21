/* =============================================
       VIZDRIVE WRO 2025 - FUTURE ENGINEERS
       OBSTACLE CHALLENGE ROUND - MAIN CODE
       
       CALIBRATION NOTICE:
       Certain parameters in this code use non-ideal values.  
       These are empirical corrections added to
       compensate for mechanical tolerances, motor asymmetry, friction, and
       sensor offsets observed during real-world testing.
 ============================================= */

#include <Wire.h> // I2C connection
#include <Adafruit_MPU6050.h> // MPU-6050
#include <Adafruit_Sensor.h>
#include <Adafruit_TCS34725.h>  // Color sensor
#include <Servo.h>  // Servo motor
#include <NewPing.h>  // Ultrasonic sensors
#include <Pixy2.h>  // Pixycam 2.1
 
// GLOBAL VARIABLES
 
// 1. Mobility Functionalities
 
// DC motor PWM pins
#define MOTOR_INA_PWM 5
#define MOTOR_INB_PWM 4
float speedBoost = 1.1;
int motorSpeed = int(175 * (speedBoost)); //175
int parkingSpeed = int(155 * (speedBoost)); //155
int parkingReverseSpeed = int(155 * (speedBoost)); //155
int backwardSpeed = int(200 * (speedBoost)); //200

// Headlight PWM pins
#define LED_HEADLIGHT_PWM 8

// Servomotor pin
#define SERVO_PIN 6
 
// Constant servo angles
const int SERVO_STRAIGHT = 101;
int SERVO_MAX = 45; // maximum angle of servo's rotation
int SERVO_LEFT  = SERVO_STRAIGHT - SERVO_MAX;
int SERVO_RIGHT = SERVO_STRAIGHT + SERVO_MAX;

// Defining the servomotor
Servo steeringServo;

int turnAngle = 72; // Defining a variable to compensate for any constant offset in every turn
bool correctionApplied = false;  // Indicates if a correction was applied
float correctionAmount = 0.0;    // Stores what was the correction (±10.0)
 
// IR encoder pin
#define ENCODER_PIN   2
volatile unsigned long encoderPulseCount = 0; // volatile variable for pulse count
 
const float WHEEL_PERIMETER = 22.0; // cm
const float PULSES_PER_REVOLUTION = 16.0; // PPR
const float PULSES_PER_CM = PULSES_PER_REVOLUTION/WHEEL_PERIMETER;  // Pulses per cm
 
// 2. PID Control and Orientation

// Defining the MPU6050 with Adafruit library
Adafruit_MPU6050 mpu;
float gyroZ_offset = 0.0; // MPU6050 calibrated offset
 
static float yaw = 0.0; // variable for accumulated yaw
static float targetYaw = 0.0; // target yaw for PID correction
const int samples = 250;  // samples for MPU6050 calibration
int sensorFailCount = 0;
const int maxSensorFails = 10;  // Número máximo de fallos antes de actuar
 
static float previousError = 0.0; // previous error for the time derivative correction
 
// PID constants
const float Kp = 1.8; // proportional gain
const float Kd = 1.2; // derivative gain
const float Ki = 0.0; // integral gain (inactive to avoid wind-up and over-correction)
 
// 3. Color Detection

// color sensor located at the bottom
const int BLUE_RED_THRESHOLD =   80;   // blue color thresholds. og = 80
const int BLUE_GREEN_THRESHOLD = 86;
const int BLUE_BLUE_THRESHOLD =  68;
const int ORANGE_RED_THRESHOLD =   112; // orange color thresholds
const int ORANGE_GREEN_THRESHOLD = 85;
const int ORANGE_BLUE_THRESHOLD =  60;

// Defining the i2c color sensor as "floorTcs"
static Adafruit_TCS34725 floorTcs(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_1X);
 
// 4. Ultrasonic Sensor Distance Measurement
 
// Left ultrasonic sensor trigger and echo pins
#define US_LEFT_TRIG 15
#define US_LEFT_ECHO 17
// Right ultrasonic sensor trigger and echo pins
#define US_RIGHT_TRIG 11
#define US_RIGHT_ECHO 13

// Front ultrasonic sensor trigger and echo pins
#define US_FRONT_TRIG 7
#define US_FRONT_ECHO 9
 
// Max detection distance for the definition of the ultrasonic sensors
#define MAX_DISTANCE 200

// Defining the ultrasonic sensors with NewPing library
NewPing sonarFront(US_FRONT_TRIG, US_FRONT_ECHO, MAX_DISTANCE);
NewPing sonarLeft(US_LEFT_TRIG, US_LEFT_ECHO, MAX_DISTANCE);
NewPing sonarRight(US_RIGHT_TRIG, US_RIGHT_ECHO, MAX_DISTANCE);

// 5. Artificial Vision with Pixycam 2.1
 
Pixy2 pixy;

// Signatures for the PixyCam color recognition
#define SIGNATURE_RED 2 //2
#define SIGNATURE_GREEN 1 //1
 
/*Configuration of the Region of Interest
Coordinates of the Pixy resolution: width: 316, height: 208*/
 
// Y-coordinate threshold for an object to be considered "near" and start evasion.
// Any part of the object below this Y-coordinate will trigger detection.
const int ROI_Y_BOUND = 208/6;
 
const int IMAGE_WIDTH = 316;
// Define sections of the image for evasion completion check
const int LEFT_SECTION_END_X = IMAGE_WIDTH / 4;   // X-coordinate separating left third
const int RIGHT_SECTION_START_X = (3 * IMAGE_WIDTH) / 4; // X-coordinate separating right third
 
// 6. State Logic

unsigned int lapTurnCount = 0;
int lapFinishedNumber = 12; // variables for lap counting
bool parkingMode = false, SystemShutdown = false; // variables for parking and shutdown

bool turningInProgress = false; // boolean that indicates when a turn was completed
unsigned long lastTurnTime = 0; // variable to control the turn cooldown
const float TURN_THRESHOLD = 3.0; // tolerance threshold for turns
unsigned long turnCorrectionTime = 3000000; // 4 seconds

bool correctionState = false;  // variable to avoid repetitive ultrasonic corrections in one lap

float turnTargetYaw = 0.0;  // target yaw for turning
int direction = 0;  // turn direction → -1: counterclockwise, +1: clockwise
 
int lastSignature = 0;  // last evaded signature for ultrasonic correction
int parkingSignature = 0;

// Miscellaneous
const int buttonPin = 3; // start button pin
const int ledPin = 23; // indicator LED
unsigned long lastUpdateOrientationTime = 0; // for main loop non-blocking delay
unsigned long lastUpdateTime = 0;
unsigned long lastCentreUpdateTime = 0; // for centring non-blocking while loop
 
void setup() {
  Serial.begin(115200); // initializing the serial monitor (115200 bauds is the Pixycam requirement)
  
  // Call all setup functions
  initMovement();
  initOrientation();
  initColorSensors();
  initUltrasonic();
  initVision();
  //initHeadlights(true, 150);
  initStateLogic();
 
  waitForStartButton(); // wait for button press
  lastUpdateTime = millis();
  exitParking();
}
 
void loop() {
  unsigned long now = millis(); // updates current time
 
  // Code executed at roughly 33.3 Hz
  if (now - lastUpdateTime < 25) return;  // if 30 ms have not passed, do not execute code
 
  //debugging();
 
  driveForward(motorSpeed); // drive forward with a specific speed
 
  updateOrientation();  // PID control and MPU6050 orientation control
  keepOrientation();  
 
  if ( lapTurnCount > 0) recentreIfNeeded();  // avoid walls with ultrasonic sensors
 
  if (detectFloorColor()) handleColorAction();  // detect color lines on the floor for turns
  if (turningInProgress) completedTurn(); // function to detect completed turns

  if (obstacleDetected()) handleEvasion();  // detect obstacles and evade accordingly
  
  if (parkingMode == true) parkingManeuver(); // park after 3 laps

  lastUpdateTime = now; // updates the time for next iteration
}
 
// Functions
 
// Function for a non-blocking delay
// Only executes the updateOrientation function
void safeDelay(unsigned long ms) {
  unsigned long startDelay = micros();  // sets the start time
  unsigned long lastUpdateTimeD = 0UL; // para controlar la frecuencia de updateOrientation

  while ((micros() - startDelay) < ms*1000UL) {
    unsigned long nowD = micros(); // updates current time
    if (nowD - lastUpdateTimeD > 25000UL) {
      updateOrientation();  // updates orientation
      Serial.print("Remaining time: "); // debugging
      Serial.print((micros() - startDelay));
      Serial.println(" µs");
      lastUpdateTimeD = nowD; // updates the time for next iteration
    }
  }
  Serial.print("Finnished Delay----------------------------");
}

// Function for a non-blockaing delay
bool safeDelayColor(unsigned long ms) {
  unsigned long startDelay = micros();  // tiempo de inicio
  unsigned long lastUpdateTimeDC = 0UL; // para controlar la frecuencia de updateOrientation

  while ((micros() - startDelay) < ms * 1000UL) {
    unsigned long nowDC = micros();

    // Solo actualiza orientación cada 25 ms (25000 µs)
    if (nowDC - lastUpdateTimeDC >= 25000UL) {
      updateOrientation();
      lastUpdateTimeDC = nowDC;
    }

    if (detectFloorColor()) {
      handleColorAction();
      return true;  
    }

    delay(3);

    Serial.print("Remaining time: ");
    Serial.print(nowDC - startDelay);
    Serial.println(" µs");
  }

  Serial.println("Finished Delay-------------------- COLOR");
  return false; // si no hubo color, devuelve false
}

// SETUP FUNCTIONS
 
// Setup for Mobility Functionalities
void initMovement() {
  // DC Motor PWM pins as OUTPUT
  pinMode(MOTOR_INA_PWM, OUTPUT);
  pinMode(MOTOR_INB_PWM, OUTPUT);
  // Attach pin to the servo
  steeringServo.attach(SERVO_PIN);
  setSteeringAngle(SERVO_STRAIGHT); // set steering angle to straight in the setup
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), encoderISR, RISING); // attach the interruption to the ISR, that counts the pulses
  Serial.println("Movement initialized...");
}
 
// Setup for Orientation Functionalities
void initOrientation() {
  Wire.begin(); // i2c connection to the MPU
  Wire.setWireTimeout(3000, true); // 3 ms timeout, reset bus on timeout
  while (!mpu.begin()) {
   Serial.println("MPU6050 not found, trying again in 3 seconds...");
   delay(3000);
  }
  Serial.println("MPU6050 initialized succesfully.");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);  // sets the accelerometer's measurement range to ±8g
  mpu.setGyroRange(MPU6050_RANGE_250_DEG); // sets the gyroscope's measurement range to ±500 degrees/second.
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ); // configures the DLPF bandwidth to 94 Hz, to reduce high-frequency noise.
  delay(250); // brief pause for stabilization
 
  // MPU6050 calibration
  float sum = 0.0;
  for (int i = 0; i < samples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sum += g.gyro.z;
    delay(5);
  }
  gyroZ_offset = sum / samples; // gets the average yaw offset in the static robot
  Serial.print("MPU Calibration completed.  Calculated offset: ");
  Serial.println(gyroZ_offset);
  Serial.println("Orientation control fully initialized...");
}
 
// Setup for Color Detection with TCS color sensors
void initColorSensors() {
  Serial.println("Iniciando sensores de color.");
  floorTcs.begin(); // start i2c tcs color sensor
  Serial.println("Color sensors initialized...");
}
 
// Setup for Ultrasonic Sensors
void initUltrasonic() {
  // No setup needed; sonars declared in the header
  Serial.println("Ultrasonic sensor initialized...");
}
 
 // Setup for PixyCam
void initVision() {
  pixy.init();  
  delay(100);
  Serial.println("Artificial vision initialized...");
}
// Setup for State Logic Functionalities
void initStateLogic() {
  setTargetYaw(0.0);
  Serial.println("State logic initialized...");
}
 
void initHeadlights(bool pixyLED, int ledBrightness) {
  pixy.setLamp(pixyLED, 1); // pixycam's lamp for ilumination
  delay(100);
  analogWrite(LED_HEADLIGHT_PWM, ledBrightness);
  delay(100);
  Serial.print("Headlights initialized...");
}

// Start button function.
// Blocks the program until the button is pressed.
void waitForStartButton() {
  pinMode(buttonPin, INPUT);  // configures the button and led pin
  pinMode(ledPin, OUTPUT);
  Serial.println("Ready for initiation. Waiting for button press...");
  digitalWrite(ledPin, HIGH); // turns on the LED indicator
 
  while (!digitalRead(buttonPin)) { // while the button is not pressed
    delay(5);
  }
 
  delay(1000);
  digitalWrite(ledPin, LOW);  // turns off the LED indicator
}
 
// Movement Functions
 
// Drive forwards at a specific speed
void driveForward(int speed) {
  analogWrite(MOTOR_INA_PWM, speed);  // sets the Motor INA to the speed PWM signal
  digitalWrite(MOTOR_INB_PWM, LOW); // sets the Motor INB to LOW (0 PWM)
}
 
// Drive backwards at a specific speed
void driveBackward(int speed) {
  analogWrite(MOTOR_INA_PWM, LOW);
  digitalWrite(MOTOR_INB_PWM, speed);
}
 
// Stops motors forcefully
void stopBrake() {
  digitalWrite(MOTOR_INA_PWM, HIGH); // sets both input pins to HIGH, to create a short-circuit braking effect
  digitalWrite(MOTOR_INB_PWM, HIGH);
}
 
// Function for controlling angular position of front steering wheels with attached servomotor
void setSteeringAngle(int angle) {
  angle = constrain(angle, SERVO_LEFT, SERVO_RIGHT);  // constrains the angle to limit the maximum and minimum servo angles
  steeringServo.write(angle); // writes the given angle to the servo motor

  /*Serial.print("Servo angle set to: "); // debugging
  Serial.print(angle);
  Serial.println("°"); // prints the servo angle to the serial monitor*/
}
 
// Encoder Interrupt Service Routine to count pulses
void encoderISR() {
  encoderPulseCount++;
}

// Function to move a specific distance (cm) while keeping orientation
void encoderDelayOrientation(unsigned long cm) {
  encoderPulseCount = 0; // Reset encoder pulse count
  int pulsesRequired = PULSES_PER_CM * cm; // Calculate required pulses
  Serial.print("PULSES REQUIRED ORIENTATION: ");
  Serial.println(pulsesRequired);

  unsigned long lastUpdateTimeEO = 0UL; // Track last orientation update

  while (1) {
    unsigned long nowEO = micros();

    // Update orientation only every 25 ms
    if (nowEO - lastUpdateTimeEO >= 25000UL) {
      updateOrientation();
      lastUpdateTimeEO = nowEO;
    }

    keepOrientation();   // Maintain orientation
    delay(3);            // Small delay to prevent CPU overload

    Serial.print("Missing pulses: ");
    Serial.println((pulsesRequired - encoderPulseCount + 2));

    // Exit loop when required pulses are reached
    if (((pulsesRequired - encoderPulseCount) + 10 <= 10) || ((pulsesRequired - encoderPulseCount) > 30000000UL)) {
      break;
    }
  }
  Serial.println("Finished Encoder Delay Orientation");
}

// Function to move a specific distance (cm) without orientation correction
void encoderDelay(unsigned long cm) {
  encoderPulseCount = 0; // Reset encoder pulse count
  int pulsesRequired = PULSES_PER_CM * cm; // Calculate required pulses
  Serial.print("PULSES REQUIRED: ");
  Serial.println(pulsesRequired);

  unsigned long lastUpdateTimeE = 0UL; // Track last orientation update

  while (1) {
    unsigned long nowE = micros();

    // Update orientation only every 25 ms
    if (nowE - lastUpdateTimeE >= 25000UL) {
      updateOrientation();
      lastUpdateTimeE = nowE;
    }

    delay(3); // Small delay to prevent CPU overload

    Serial.print("Missing pulses: ");
    Serial.println((pulsesRequired - encoderPulseCount + 2));

    // Exit loop when required pulses are reached
    if (((pulsesRequired - encoderPulseCount) + 10 <= 10) || ((pulsesRequired - encoderPulseCount) > 30000000UL)) {
      break;
    }
  }
  Serial.println("Finished Encoder Delay");
}

// Function for movement with obstacle detection and evasion
bool safeDelayEvasionEncoder(unsigned long cm) {
  encoderPulseCount = 0;  // Reset encoder pulse count
  int pulsesRequired = PULSES_PER_CM * cm; // Calculate required pulses
  Serial.print("PULSES REQUIRED: ");
  Serial.println(pulsesRequired);

  bool obstacleFound = false;
  unsigned long lastUpdateTimeEE = 0UL; // Track last orientation update

  while (1) {
    unsigned long nowEE = micros();

    // Update orientation only every 25 ms
    if (nowEE - lastUpdateTimeEE >= 25000UL) {
      updateOrientation();
      lastUpdateTimeEE = nowEE;
    }

    keepOrientation(); // Maintain orientation
    delay(5);

    // If obstacle detected, handle evasion
    if (obstacleDetected()) {
      Serial.print("An obstacle was detected during evasionDelay");
      handleEvasion();
      obstacleFound = true;
      Serial.println("obstacleFound = TRUE");
    }

    Serial.print("Remaining steps: ");
    Serial.print((pulsesRequired - encoderPulseCount + 2));
    Serial.println(" pulses");

    delay(5);

    // Exit loop when required pulses are reached
    if (((pulsesRequired - encoderPulseCount) + 10 <= 10) || ((pulsesRequired - encoderPulseCount) > 30000000UL)) {
      break;
    }
  }

  Serial.println("Finished Delay Encoder");
  return obstacleFound;
}

// Parallel parking maneuver
void parkingManeuver() {
  delay(100);
  // The parking is near
  if (direction == 1) {
    setSteeringAngle(SERVO_STRAIGHT); // Resets Servo Angle
    //Defining math variables
    int distance = getDistance(sonarLeft);
    int a_cm = distance - 43;
    int b_cm = 75;
    int c_cm;
    int angle;
    int turnDirection;

    Serial.print(a_cm);
    Serial.println(" - Step1Done (a_cm)...");
    // Checking turn direction
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
    Serial.print("TurnDirection: ");
    Serial.print(turnDirection);
    Serial.print(" --- c_cm: ");
    Serial.print(c_cm);
    Serial.println(" - Step2Done");

    if (turnDirection != 0) { // If diagonal movement is required
      float tangent = ((float)abs(a_cm)/b_cm); // Calculate tangent
      Serial.print("Tan: ");
      Serial.print(tangent);
      angle = (int)(atan(tangent) * (180.0 / M_PI)); // Calculate required turn angle
      Serial.print(" --- TanAngle: ");
      Serial.print(angle);
      turnTargetYaw = (turnTargetYaw + (angle*direction*turnDirection));  // Sets the target yaw to the required angle, according to direction required to turn
      setTargetYaw(turnTargetYaw); // Applies the target yaw
      Serial.print("TargetYaw: ");
      Serial.print(turnTargetYaw);
    } else { // No angle was required
      Serial.print(" - No angle required - ");
    }

    Serial.println(" --- Step3Done");
    // Parking
    driveForward(parkingSpeed);
    Serial.println("Accelerating");

    encoderDelayOrientation(c_cm); // Waits until the defined distance is complete
    stopBrake();  // Stops the motors

    if (turnDirection != 0) { // Resets angles when distance is complete
      turnTargetYaw = (turnTargetYaw - (angle*direction*turnDirection));  // Resets the target yaw
      setTargetYaw(turnTargetYaw); // Applies the target yaw
    }
      
    driveForward(parkingSpeed);
    encoderDelayOrientation(48); // Waits until the defined distance is complete
    stopBrake();  // stops the motors
    delay(100);

    setSteeringAngle(SERVO_LEFT); // parking maneuver
    driveBackward(parkingReverseSpeed);
    encoderDelay(34); // Waits until the defined distance is complete
    stopBrake();  // stops the motors

    setSteeringAngle(SERVO_RIGHT);
    driveBackward(parkingReverseSpeed);
    encoderDelay(36); // Waits until the defined distance is complete
    stopBrake();
    setSteeringAngle(SERVO_STRAIGHT);
    while (1);  // stops the program
  }
  // Parking is far
  if (direction == -1) {
    delay(100);
    setSteeringAngle(SERVO_STRAIGHT);
    // Defines math variables
    int distance = getDistance(sonarRight);
    int a_cm = distance - 30;
    int b_cm = 42;
    int c_cm;
    int angle;
    int turnDirection;
    
    Serial.print(a_cm);
    Serial.println(" - Step1Done");

    if  (a_cm > 0) { // Far from parking
      c_cm = sqrt(pow(a_cm, 2) + pow(b_cm, 2)); // Calculating diagonal distance
      turnDirection = 1;
    }
    else if (a_cm < 0){ // Near to parking
      c_cm = sqrt(pow(a_cm, 2) + pow(b_cm, 2)); // Calculating diagonal distance
      turnDirection = -1;
    } else { // Perfect distance
      c_cm = b_cm; // No diagonal distance
      turnDirection = 0;
    }
    Serial.print("TurnDirection: ");
    Serial.print(turnDirection);
    Serial.print(" --- c_cm: ");
    Serial.print(c_cm);
    Serial.println(" - Step2Done");

    if (turnDirection != 0) { // If diagonal movement is required
      float tangent = ((float)abs(a_cm)/b_cm); // Calculates tanglent
      Serial.print("Tan: ");
      Serial.print(tangent);
      angle = (int)(atan(tangent) * (180.0 / M_PI)); // Calculates required steering angle
      Serial.print(" --- TanAngle: ");
      Serial.print(angle);
      turnTargetYaw = (turnTargetYaw + (angle*direction*turnDirection));  // Sets the target yaw to required angle, according to direction
      setTargetYaw(turnTargetYaw); // Applies target yaw
      Serial.print(" - TargetYaw: ");
      Serial.print(turnTargetYaw);
    } else { // No angle was required
      Serial.print(" - No angle required - ");
    }
    Serial.println(" --- Step3Done");
    // Parking
    driveForward(parkingSpeed);
    Serial.print("accelerating"); 

    encoderDelayOrientation(c_cm); // Waits until the defined distance is complete
    //digitalWrite(ledPin, LOW);
    stopBrake();  // stops the motors

    if (turnDirection != 0) {
      turnTargetYaw = (turnTargetYaw - (angle*direction*turnDirection));  // sets the target yaw to ± 90°, according to direction
      setTargetYaw(turnTargetYaw);
    }
    // Distance to travel for block signature

    driveForward(parkingSpeed);
    encoderDelayOrientation(135);

    //encoderDelayOrientation(requiredDistanceForSignature); // Waits until the defined distance is complete
    stopBrake();  // stops the motors
    delay(100);

    setSteeringAngle(SERVO_RIGHT); // parking maneuver
    driveBackward(parkingReverseSpeed);
    encoderDelay(34); // Waits until the defined distance is complete
    stopBrake();  // stops the motors

    setSteeringAngle(SERVO_LEFT);
    driveBackward(parkingReverseSpeed);
    encoderDelay(37); // Waits until the defined distance is complete

    stopBrake();
    setSteeringAngle(SERVO_STRAIGHT);
    while (1);  // stops the program
  }
}


// Orientation Functions
 
// Function to get the accumulated yaw angle
void updateOrientation() {
  unsigned long now = micros();
  float dt = (now - lastUpdateOrientationTime) / 1000000.0;  // segundos

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);  // Obtener los datos del sensor

  float gyroZ = g.gyro.z;

  // Validar que la lectura del giroscopio Z sea válida
  if (!isnan(gyroZ) && abs(gyroZ) < 10.0) {  // Filtra valores absurdos
    sensorFailCount = 0;  // Resetear contador de fallos

    float gyroZ_deg = (gyroZ - gyroZ_offset) * 57.295779513;

    if (abs(gyroZ_deg) > 0.1) {  
      yaw += gyroZ_deg * dt;  
    }

  } else {
    sensorFailCount++;
    Serial.print("Lectura inválida del giroscopio (Z): ");
    Serial.println(gyroZ, 6);

    if (sensorFailCount >= maxSensorFails) {
      Serial.println("Demasiados fallos consecutivos del MPU. Verifica conexión o reinicia.");
      mpu.begin();
      sensorFailCount = 0;
    }
  }

  lastUpdateOrientationTime = now;  // Siempre actualizar el tiempo
}
 
// Function to apply the PID control for orientation correction
void keepOrientation() {
  float error = targetYaw - yaw;  // calculates the error in orientation
  float derivative = error - previousError; // error change over time
  int correction = SERVO_STRAIGHT - (int)(Kp * error + Kd * derivative);  // correction applying PID control
  setSteeringAngle(correction); // sets the servo angle to correct the error
  previousError = error;  // updates previous error
}
 
// Function to change the target orientation for PID correction
void setTargetYaw(float angle_deg) {
  targetYaw = angle_deg;
}
 
// Function that returns the current accumulated yaw
float getCurrentYaw() {
  return yaw;
}
 
// Function that returns the absolute value of the difference between the target yaw and yaw.
float getAbsoluteYawError() {
  return abs(abs(targetYaw)-abs(yaw));
}
 
// Color Functions
 
// Function for orange and blue floor lines detection
bool detectFloorColor() {
  if (lapTurnCount == lapFinishedNumber) return false;
  uint16_t r,g,b,c;
  floorTcs.getRawData(&r,&g,&b,&c); // gets raw rgbc data from the sensor
  if (!c) return false; // avoid dividing by 0
  int rn=r*255/c, gn=g*255/c, bn=b*255/c; // normalizes values with the total light (c), for different light levels adaptability
 
  // Debugging for color sensor
  /*Serial.print("R: ");  Serial.println(rn);
  Serial.print("    G: ");  Serial.print(gn);
  Serial.print("    B: ");  Serial.println(bn);
  Serial.println("-------------------------------");*/
 
  // For the first lap turn, the first detected color defines the direction:
  if (lapTurnCount == 0 && bn>BLUE_BLUE_THRESHOLD && rn < BLUE_RED_THRESHOLD) {  // if blue color is detected
    direction = -1; // blue: counter-clockwise (-1)
    turnAngle = 74;
    /*Serial.print("Direction set to: ");  // debugging
    Serial.println(direction);*/
    return true;
  } else if (lapTurnCount == 0 && rn>ORANGE_RED_THRESHOLD && gn<ORANGE_GREEN_THRESHOLD) { // if orange color is detected
    direction = +1; // orange: clockwise (+1)
    turnAngle = 74;
    /*Serial.print("Direction set to: ");  // debugging
    Serial.println(direction);*/
    return true;
  }

  if (direction<0 && bn>BLUE_BLUE_THRESHOLD && rn < BLUE_RED_THRESHOLD ) {
    // Serial.println("BLUE color detected");
    resetYawCorrection();
    return true;
    } // returns true if a color detection matches the threshold
  if (direction>0 && rn>ORANGE_RED_THRESHOLD && gn<ORANGE_GREEN_THRESHOLD) {
    // Serial.println("ORANGE color detected");
    resetYawCorrection();
    return true;
    }
  return false;
}
 
// Function for turns when a color is detected
void handleColorAction() {
  if (turnCooldown() && !turningInProgress) { // if a color is detected at the bottom, and there's no turning in progress...
    lapTurnCount++; // lap turn counting
    turningInProgress = true;
    digitalWrite(ledPin, HIGH); // turns on the LED for visualization of states

    // Drives forward until the robot is near to the wall
    setSteeringAngle(SERVO_STRAIGHT);
    int df = sonarFront.ping_cm();
    safeDelay(10);
 
    while (df > 25 || df == 0) {  // while the distance to the wall is greater than 10
      df = getDistance(sonarFront); // updates the front distance
      keepOrientation();  // keeps the orientation
      safeDelay(10);  // brief delay
 
      Serial.print("Distance to the wall: ");  // debugging
      Serial.println(df);
    }
    stopBrake();  // stops the motors
   
    NewPing sonar = (direction < 0) ? sonarRight : sonarLeft; // according to the direction, decides which ultrasonic sensor to use
    int distance = getDistance(sonar);  // gets the distance to the wall, and acts accordingly

    // Si el primer intento falla (es 0), reintenta hasta 2 veces más
if (distance == 0) {
  // Bucle para los reintentos (se ejecuta un máximo de 2 veces)
  for (int i = 0; i < 2; i++) {
    distance = getDistance(sonar); // Reintenta la medición
    if (distance != 0) {
      break; // Si se obtiene una medida válida, sale del bucle
    }
  }
}



    safeDelay(50);

    if (distance < 15) {  // if the distance to the wall is short,
      int addition;
      if(lapTurnCount == 4 || lapTurnCount == 8 || lapTurnCount == 12){
        addition = 16;
      } else {
        addition = 14;
      }
      turnTargetYaw = (turnTargetYaw + ((turnAngle + addition) * -direction));  // sets the target yaw to ± 90°, according to direction
      setTargetYaw(turnTargetYaw);

      setSteeringAngle(SERVO_STRAIGHT);
      safeDelay(50);
      driveBackward(motorSpeed);  // drives backwards in a calculated maneuver
      safeDelay(1000);
      stopBrake();
      safeDelay(100);
      driveForward(200);  // drives forward until it completes the turn
      
      while(turningInProgress) {  // while the robot is turning,
        keepOrientation();  // updates the orientation
        if (turningInProgress && getAbsoluteYawError() < 3) {
          digitalWrite(ledPin, LOW); // turns off the LED for visualization of states
          turningInProgress = false;
          lastSignature = 0;
 
          lastTurnTime = micros();
        }
        safeDelay(5);   // safeDelay updates the orientation
      }
      if(lapTurnCount == 4 || lapTurnCount == 8 || lapTurnCount == 12)   {
        driveBackward(backwardSpeed); // drives backwards to collide against the wall
        setSteeringAngle(SERVO_STRAIGHT);
        safeDelay(2000);
        yaw = turnTargetYaw;  // resets the yaw
        setSteeringAngle(SERVO_STRAIGHT);      
        driveForward(motorSpeed); // continues to drive forward
        safeDelay(500);
        if (lapTurnCount == 12) {
          parkingMode = true;
          return;
        } else {
          return;
        }
      }
      return;

    } else {  // if the robot is far from the wall
      int addition;
      if(lapTurnCount == 4 || lapTurnCount == 8 || lapTurnCount == 12){
        addition = 2;
      } else {
        addition = 0;
      }
      turnTargetYaw = (turnTargetYaw + ((turnAngle + addition) * -direction));  // sets the target yaw to ± 90°, according to direction
      setTargetYaw(turnTargetYaw);

      int angle = (direction < 0) ? SERVO_RIGHT+10 : SERVO_LEFT-10; // sets the servo angle according to the direction
      setSteeringAngle(SERVO_STRAIGHT); // drives backwards for 500 ms
      safeDelay(50);
      stopBrake();
      safeDelay(250);
      steeringServo.write(angle); // sets the servo to the modified sharper angle
      safeDelay(150);
      driveBackward(backwardSpeed); // drive backwards until the robot corrects its orientation

      while(turningInProgress) {  // while the robot is turning,
        // keepOrientation();  // updates the orientation
        completedTurn();  // verifies if the turn was completed
        safeDelay(5);   // safeDelay updates the orientation
      }

      if(lapTurnCount == 4 || lapTurnCount == 8 || lapTurnCount == 12) {
        setSteeringAngle(SERVO_STRAIGHT);
        driveBackward(backwardSpeed);
        safeDelay(1800);  // drives backwards to collide against the wall
        yaw = turnTargetYaw;  // and resets the yaw
        driveForward(motorSpeed);
        safeDelay(500);
        return;
      } else if (distance > 60) { 
        setSteeringAngle(SERVO_STRAIGHT);
        driveBackward(backwardSpeed);
        safeDelay(800);
        turnTargetYaw = (turnTargetYaw + ((13) * -direction));  // sets the target yaw to ± 90°, according to direction
        setTargetYaw(turnTargetYaw);
        return;
      } else {
        turnTargetYaw = (turnTargetYaw + ((13) * -direction));  // sets the target yaw to ± 90°, according to direction
        setTargetYaw(turnTargetYaw);
        return;
      }
    }
   
  } else {
      return;
    }
}
 
void recentreIfNeeded() {
  if (correctionApplied) return;  // already applied a correction

  if (lastSignature == SIGNATURE_RED) {
    correctionAmount = 8.0;
    setTargetYaw(targetYaw + correctionAmount);
    correctionApplied = true;
    // Serial.println("Recentering after RED evasion: 8° yaw");
  } 
  else if (lastSignature == SIGNATURE_GREEN) {
    correctionAmount = -14.0;
    setTargetYaw(targetYaw + correctionAmount);
    correctionApplied = true;
    // Serial.println("Recentering after GREEN evasion: +10° yaw");
  }
}

void resetYawCorrection() {
  if (!correctionApplied) return;

  setTargetYaw(targetYaw - correctionAmount);  // Reverts the correction
  /*Serial.print("Yaw correction reverted: ");
  Serial.println(-correctionAmount);*/
  correctionApplied = false;
  correctionAmount = 0.0;
}

void exitParking() {
  int distanceR = getDistance(sonarRight);
  int distanceL = getDistance(sonarLeft);

  if (distanceR < 30 && distanceR != 0) { // if the right ultrasonic sensor is close to the wall,
    steeringServo.write(SERVO_STRAIGHT-40); // steers sharply to the left side
    driveForward(motorSpeed);
    safeDelay(1100);
  } 
  else if (distanceL < 30 && distanceL != 0) { // if the left ultrasonic sensor is close to the wall,
    steeringServo.write(SERVO_STRAIGHT+40); // steers sharply to the right side
    driveForward(motorSpeed);
    safeDelay(1400);
  } else {
    return;
  }
}
 

void completedTurn() {
  if (turningInProgress && getAbsoluteYawError() < TURN_THRESHOLD) {
    digitalWrite(ledPin, LOW); // turns off the LED for visualization of states
    turningInProgress = false;
    lastSignature = 0;
 
    lastTurnTime = micros();
 
    Serial.println("Turn completed...");
 

    if (lapTurnCount == lapFinishedNumber) { // if 3 laps are completed (4 lap turns = 1 lap) manages the shutdown
      parkingMode = true;
    }
  }
}

// Function for a cooldown between turns, to avoid repetitions
bool turnCooldown() {
  if ((micros() - lastTurnTime) > turnCorrectionTime || lapTurnCount == 0) {
    return true;
  }
  return false;
}
 
// Ultrasonic Sensors Functions
 
// Function to get distance, with a median filter for precision
int getDistance(NewPing& sonar) { // uses the original sonar object
  const int NUM_SAMPLES = 5;  // number of readings
  const int MAX_VALID_DISTANCE = 100;  // maximum distance in cm
  const int MIN_VALID_DISTANCE = 2; // minimum distance in cm
  const int STABILITY_THRESHOLD = 10; // disregards values outside the average range
 
  int samples[NUM_SAMPLES]; // defines an array to store readings
  int validCount = 0; // counts valid readings
  int sum = 0;  // variable to sum the three the valid readings
 
  for (int i = 0; i < NUM_SAMPLES; i++) {
    int d = sonar.ping_cm();  // takes three readings
    safeDelay(5); // brief delay to avoid interferences
   
    // Only accepts readings in the desired range
    if (d >= MIN_VALID_DISTANCE && d <= MAX_VALID_DISTANCE) {
      samples[validCount++] = d;  
      sum += d;
    }
  }
 
  if (validCount == 0) return 0;  // if no valid readings
 
  // Calculates the average readings
  int avg = sum / validCount;
 
  // Second filter: discards outliners
  sum = 0;  // restarts the sum to 0, to calculate a new average
  int filteredCount = 0;  // counts the readings that pass the second filter
  for (int i = 0; i < validCount; i++) {  // considers only the valid readings in the first filter
    if (abs(samples[i] - avg) <= STABILITY_THRESHOLD) { // calculates the absolute difference between the reading and the average
      sum += samples[i];  // if the difference is less than the STABILITY_THRESHOLD, it passes the second filter
      filteredCount++;
    }
  }
 
  if (filteredCount == 0) return 0; // if no readings passed the seconds filter, return 0
 
  return sum / filteredCount; // if there are many readings that passed the filter, returns the average
}
 
// Artificial Vision Functions
 
// Checks if any part of any detected block is within the crucial lower region (below ROI_Y_BOUND).
bool obstacleDetected() {
  pixy.ccc.getBlocks(); // Get the latest blocks from PixyCam.
 
  if (pixy.ccc.numBlocks == 0) {
    // Serial.println("No blocks detected."); // debugging no blocks at all
    return false;
  }
 
  // Iterate through all detected blocks to see if any are in the lower region of interest.
  for (int i = 0; i < pixy.ccc.numBlocks; i++) {
    // Check if the bottom edge of the block is below the ROI_Y_BOUND
    if (pixy.ccc.blocks[i].m_y + (pixy.ccc.blocks[i].m_height / 2) > ROI_Y_BOUND) {
      // Serial.print("Block detected in the ROI (Y:"); // Debugging: Block found in lower region.
      // Serial.print(pixy.ccc.blocks[i].m_y);
      // Serial.println(")");
      resetYawCorrection();
      return true; // A block is "near" enough to initiate evasion.
    }
  }
  // Serial.println("No blocks in the lower region."); // Debugging: No blocks in the lower region.
  return false;
}
 
// Manages the obstacle evasion process with persistent state.
void handleEvasion() {
  // 1. Check if an obstacle is near enough to start the evasion process.
  // If not, do nothing and exit the function.
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
 
    // 3. If a relevant block is found, decide the action.
    if (selectedIndex != -1) {
      int currentSignature = pixy.ccc.blocks[selectedIndex].m_signature;
      int currentX = pixy.ccc.blocks[selectedIndex].m_x;
 
      // 4. Lock onto the signature and set the steering direction if it's the first frame of evasion.
      if (activeSignature == 0) {
        activeSignature = currentSignature;
        if (activeSignature == SIGNATURE_RED) {
          setSteeringAngle(SERVO_RIGHT); // Evade red to the right.
          lastSignature = SIGNATURE_RED;
          Serial.println("Obstacle is RED. Evading RIGHT.");
        } else if (activeSignature == SIGNATURE_GREEN) {
          if (lapTurnCount == lapFinishedNumber && direction == +1)  return;
          setSteeringAngle(SERVO_LEFT); // Evade green to the left.
          lastSignature = SIGNATURE_GREEN;
          Serial.println("Obstacle is GREEN. Evading LEFT.");
        } else {
          setSteeringAngle(SERVO_STRAIGHT); // Unknown color, go straight.
          lastSignature = 0;
          Serial.println("Unknown signature. Aborting evasion.");
          break; // Exit if signature is not recognized.
        }
      }
     
      // 5. Check for evasion completion. This is the new, crucial exit condition.
      // The loop only breaks if the detected block is in the designated "safe area".
      if (activeSignature == SIGNATURE_RED && currentX < LEFT_SECTION_END_X) {
        Serial.println("Evasion successful. Red block is now on the left.");

          if (safeDelayColor(400)) break;

          setSteeringAngle(SERVO_LEFT);
          if (getAbsoluteYawError() > 70){
            if (safeDelayColor(1200)) break;
          } else if (getAbsoluteYawError() > 35){
            if (safeDelayColor(900)) break;
          } else {
            if (safeDelayColor(450)) break;
          }

        break; // Exit the while loop.
      } else if (activeSignature == SIGNATURE_GREEN && currentX > RIGHT_SECTION_START_X) {
        Serial.println("Evasion successful. Green block is now on the right.");
        
          if (safeDelayColor(400)) break;

          setSteeringAngle(SERVO_RIGHT);
          if (getAbsoluteYawError() > 70){
            if (safeDelayColor(1200)) break;
          } else if (getAbsoluteYawError() > 35){
            if (safeDelayColor(900)) break;
          } else {
            if (safeDelayColor(500)) break;
          }


        break; // Exit the while loop.
      }
    }
    // 6. If no block is detected in the "near" zone (selectedIndex remains -1), DO NOTHING.
    // The car will continue steering in the direction set previously (e.g., SERVO_RIGHT or SERVO_LEFT).
    // This handles frames where the camera temporarily loses sight of the obstacle, preventing the loop from exiting prematurely.
  }
 
  // Once the loop is broken, the evasion is considered complete.
  // The car will go back to driving straight in the next main loop iteration.
  Serial.println("Evasion Completed");
}
 
// State Logic Functions
 
void debugging() {
  Serial.print("Target yaw: ");   Serial.print(targetYaw);
  Serial.print("    Current Yaw: ");  Serial.println(yaw);
  Serial.print("Lap turns: ");  Serial.println(lapTurnCount);
  Serial.print("Motor speed: ");  Serial.println(motorSpeed);
  Serial.print("Turning in progress: "); Serial.print(turningInProgress);
  Serial.print("    Correction state: "); Serial.println(correctionState);
}

void sonarDebugging() {
  Serial.print("                    Delantero: ");
  Serial.println(sonarFront.ping_cm());

  Serial.print("Izquierda: ");
  Serial.print(sonarLeft.ping_cm());

  Serial.print("                                   Derecha: ");
  Serial.println(sonarRight.ping_cm());

  Serial.println("--------------------------------------------------------------------");
}
