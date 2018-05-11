#define USE_ARDUINO_INTERRUPTS true

#include <Servo.h>
#include <PulseSensorPlayground.h>

const int PIN_INPUT = A0; // HRM
const int THRESHOLD = 550;   // Adjust this number to avoid noise in HRM when idle
PulseSensorPlayground pulseSensor;
float restingHeartRate = 70.0;
float anxietyHRBoundary;
float avgHeartRate = 70.0;
unsigned long int lastSpinTime = 0;

// Manually entering user's stats
int userWeightlbs = 130;
int heightInInches = 68;

// These are our motors.
Servo myMotor;
Servo myMotor2;

void setup() {
  /*
     Use 115200 baud because that's what the Processing Sketch expects to read,
     and because that speed provides about 11 bytes per millisecond.

     If we used a slower baud rate, we'd likely write bytes faster than
     they can be transmitted, which would mess up the timing
     of readSensor() calls, which would make the pulse measurement
     not work properly.
  */
 
  pinMode(PIN_INPUT,  INPUT);
  Serial.begin(115200);

  // Configure the PulseSensor manager.
  pulseSensor.begin();
  pulseSensor.analogInput(PIN_INPUT);
  pulseSensor.setSerial(Serial);
  pulseSensor.setThreshold(THRESHOLD);

  // Get a starting resting heart rate. Do it for 10 seconds.
  unsigned long startTime = millis();
  float heartRate = 0;

  while (millis() - startTime < 10000) {
    heartRate = (float) pulseSensor.getBeatsPerMinute();

    // Get rid of any potential trash data. Tighten these
    // bounds if necessary.
    if (heartRate > 40 && heartRate < 160) {
      restingHeartRate = (0.95 * restingHeartRate) + (0.05 * heartRate);
    }
  }

  anxietyHRBoundary = getAnxietyHRBoundary(restingHeartRate);

  Serial.print("Your resting heart rate is: ");
  Serial.println(restingHeartRate);
  Serial.print("Your anxiety HR boundary is: ");
  Serial.println(anxietyHRBoundary);

  /* Motor setup */
  // Put the motor to Arduino pin #9 and pin #10
  myMotor.attach(9);
  myMotor2.attach(10);

  // Print a startup message
  Serial.println("initializing");
  armMotor();
}

void loop() {
  int reading = analogRead(0); 
  delay(20);

  avgHeartRate = (0.95 * avgHeartRate) + (0.05 * pulseSensor.getBeatsPerMinute());

  if (avgHeartRate > anxietyHRBoundary) {
    if (millis() - lastSpinTime > 30000) {
      Serial.println(avgHeartRate);
      spinMotor(3000);
      delay(2000);
      armMotor();
      lastSpinTime = millis();
    }
  }
}

void armMotor() {
  myMotor.write(0);
  myMotor2.write(0);
  delay(500);
  myMotor.write(5);
  myMotor2.write(5);
  delay(500);
  myMotor.write(10);
  myMotor2.write(10);
  delay(500);
  myMotor.write(15);
  myMotor2.write(15);
  delay(500);
  myMotor.write(20);
  myMotor2.write(20);
  delay(2000);
}

void spinMotor(int timeInterval) {
  unsigned long int startTime = millis();

  myMotor.write(50);
  myMotor2.write(50);
  
  while (millis() - startTime < timeInterval) {
    delay(500);
  }

  myMotor.write(0);
  myMotor2.write(0);
}

float getAnxietyHRBoundary(float restingHR) {
  float userBMI = (userWeightlbs/(heightInInches*heightInInches)) * 703;
  Serial.print("Your BMI is: ");
  Serial.println(userBMI);

  // Multipliers increase with BMI because people with lower BMIs have
  // lower variance of HR.
  if (userBMI > 30) {
    return restingHR * 1.3;
  } else if (userBMI > 20) {
    return restingHR * 1.2;
  } else if (userBMI > 18.5) {
    return restingHR * 1.1;
  } else {
    return restingHR * 1.05;


  }
}

