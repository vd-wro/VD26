#include "NewPing.h"

#define US_LEFT_TRIG 47
#define US_LEFT_ECHO 28
#define US_RIGHT_TRIG 9
#define US_RIGHT_ECHO 10
#define MAX_DISTANCE 200

NewPing sonarLeft(US_LEFT_TRIG, US_LEFT_ECHO, MAX_DISTANCE);
NewPing sonarRight(US_RIGHT_TRIG, US_RIGHT_ECHO, MAX_DISTANCE);

void setup() {
  Serial.begin(115200);
  Serial.println("ultrasónicos");

}

void loop() {
  int dl = getDistance(sonarLeft);
  int dr = getDistance(sonarRight);

  Serial.print("Right: ");
  Serial.print(dr);
  Serial.print("    Left: ");
  Serial.println(dl);
}

int getDistance(NewPing& sonar) {
  const int NUM_SAMPLES = 5;
  const int MAX_VALID_DISTANCE = 80;  // cm
  const int MIN_VALID_DISTANCE = 4;   // cm
  const int STABILITY_THRESHOLD = 10; // cm
  
  int samples[NUM_SAMPLES];
  int validCount = 0;
  int sum = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    int d = sonar.ping_cm();
    delay(3);

    if (d >= MIN_VALID_DISTANCE && d <= MAX_VALID_DISTANCE) {
      samples[validCount++] = d;
      sum += d;
    }
  }

  if (validCount == 0) return 0;

  int avg = sum / validCount;

  sum = 0;
  int filteredCount = 0;
  for (int i = 0; i < validCount; i++) {
    if (abs(samples[i] - avg) <= STABILITY_THRESHOLD) {
      sum += samples[i];
      filteredCount++;
    }
  }

  if (filteredCount == 0) return 0;

  return sum / filteredCount;
}