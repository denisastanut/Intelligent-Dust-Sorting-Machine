#include <LiquidCrystal.h>
#include <Servo.h>

// Pin definitions
const int rs = 11, en = 12, d4 = 10, d5 = 9, d6 = 8, d7 = 7, ct = 13;
const int proximitySensorPin = 5;
const int raindropSensorPin = A0;
const int irSensorPin = 6;
const int servoPin = 4;
const int servoPinDust = 3;
const int buzzerPin = 2;
const int buttonPin = 1;


LiquidCrystal mylcd(rs, en, d4, d5, d6, d7);

Servo myservo;
Servo dustServo;


int dryCount = 0;
int wetCount = 0;
int metalCount = 0;
int totalCount = 0;

bool showPercentages = false;

byte happyFace[8] = {
  B00000,
  B01010,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};

void setup() {
  myservo.attach(servoPin);
  dustServo.attach(servoPinDust);

  myservo.write(90);
  dustServo.write(90);
  delay(1000);

  analogWrite(ct, 50);

  mylcd.begin(16, 2);
  mylcd.createChar(0, happyFace);

  mylcd.clear();

  mylcd.setCursor(0, 0);
  mylcd.print("Hello");
  mylcd.setCursor(6, 0);
  mylcd.write(byte(0));

  // Buzzer
  int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};
  int noteDurations[] = {4, 4, 4, 4, 4, 4, 4, 4};

  for (int i = 0; i < 8; i++) {
    int noteDuration = 1000 / noteDurations[i]
    tone(buzzerPin, melody[i], noteDuration);
    delay(noteDuration * 1.3);
    noTone(buzzerPin); 
  }

  delay(2000);
  mylcd.clear();

  pinMode(proximitySensorPin, INPUT);
  pinMode(raindropSensorPin, INPUT);
  pinMode(irSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}



void beep(int duration, int frequency) {
  tone(buzzerPin, frequency, duration);
  delay(duration);
  noTone(buzzerPin);
}


void displayCounts() {
  mylcd.clear();
  mylcd.setCursor(0, 0);
  mylcd.print("Dry: ");
  mylcd.print(dryCount);

  mylcd.setCursor(0, 1);
  mylcd.print("Wet: ");
  mylcd.print(wetCount);
  mylcd.print(" Metal: ");
  mylcd.print(metalCount);
}


void displayPercentages() {
  static bool showMetal = false;
  mylcd.clear();

  if (!showMetal) {
    mylcd.setCursor(0, 0);
    mylcd.print("Dry waste: ");
    if (totalCount > 0) {
      mylcd.print((float(dryCount) / totalCount) * 100, 1);
    } else {
      mylcd.print("0.0");
    }
    mylcd.print("%");

    mylcd.setCursor(0, 1);
    mylcd.print("Wet waste: ");
    if (totalCount > 0) {
      mylcd.print((float(wetCount) / totalCount) * 100, 1);
    } else {
      mylcd.print("0.0");
    }
    mylcd.print("%");

  } else {
    mylcd.setCursor(0, 0);
    mylcd.print("Metal waste:");

    mylcd.setCursor(0, 1);
    if (totalCount > 0) {
      mylcd.print((float(metalCount) / totalCount) * 100, 1);
    } else {
      mylcd.print("0.0");
    }
    mylcd.print("%");
  }

  showMetal = !showMetal;

  delay(2000);
}


void loop() {
  int irSensorState = digitalRead(irSensorPin);

  if (irSensorState == LOW) {
    mylcd.clear();
    mylcd.setCursor(0, 0);
    mylcd.print("Object detected");

    // Buzzer beep 
    beep(200, 1000);

    // Move conveyor
    mylcd.setCursor(0, 1);

    myservo.write(180);
    delay(3000); 
    myservo.write(90);
    delay(1000);

    // Read raindrop sensor value
    mylcd.clear();
    mylcd.setCursor(0, 0);
    mylcd.print("Analyzing...");

    int raindropValue = analogRead(raindropSensorPin);
    
    mylcd.setCursor(0, 1);
    mylcd.print("Value: ");
    mylcd.print(raindropValue);
    delay(1000);

    // Metal
    int proximityState = digitalRead(proximitySensorPin);

    if (proximityState == LOW) { // Metal detected
      mylcd.clear();
      mylcd.setCursor(0, 0);
      mylcd.print("Metal detected");
      metalCount++;
      totalCount++;

      // Buzzer beep for metal
      beep(500, 2000); // Long beep -> 2kHz

      // Move dustbin to metal position
      dustServo.write(90);
      delay(1000);

      myservo.write(180);
      delay(3500);
      myservo.write(90);
      delay(1000);

    } else if (raindropValue > 500) { // Wet waste detected
      mylcd.clear();
      mylcd.setCursor(0, 0);
      mylcd.print("Dry waste");
      dryCount++;
      totalCount++;

      // Buzzer beep for dry waste
      beep(300, 1500); // Medium beep -> 1.5kHz

      // Move dustbin to dry waste position
      dustServo.write(120);
      delay(600);

      dustServo.write(90);
      delay(1000);

      myservo.write(180);
      delay(3500);
      myservo.write(90);
      delay(1000);

      dustServo.write(60);
      delay(300);

      dustServo.write(90);
      delay(1000);

    } else { // Wet waste detected
      mylcd.clear();
      mylcd.setCursor(0, 0);
      mylcd.print("Wet waste");
      wetCount++;
      totalCount++;

      // Buzzer beep for wet waste
      beep(300, 1000); // Medium beep -> 1kHz

      // Move dustbin to wet waste position
      dustServo.write(60);
      delay(400);

      dustServo.write(90);
      delay(1000);

      myservo.write(180);
      delay(3500);
      myservo.write(90);
      delay(1000);

      dustServo.write(120);
      delay(600);

      dustServo.write(90);
      delay(1000);
    }

    // Reset dustbin position
    dustServo.write(90);
    delay(1000);

    mylcd.setCursor(0, 1);

  }
  
  // Show percentages or count
  if (digitalRead(buttonPin) == LOW) {
    showPercentages = !showPercentages;
    delay(100);
  }

  if (showPercentages) {
    displayPercentages();
  } else {
    displayCounts();
  }

  delay(500);
}
