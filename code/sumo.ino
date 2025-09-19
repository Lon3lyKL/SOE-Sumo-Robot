#include <Bluepad32.h>
#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>

Servo leftServo;
Servo rightServo;

String Lservo_status = "OFF";
String Rservo_status = "OFF";

const int leftServo_pwm = 23;
const int rightServo_pwm = 22;
const int Lmotor1 = 33;
const int Lmotor2 = 25;
const int Rmotor1 = 26;
const int Rmotor2 = 27;
const int button = 16;
const int strip = 14;
const int num_pixels = 3;

int RStickX = 0;
int LStickY = 0;
int L2 = 0;
int R2 = 0;
int lives = 3;
double theta = 0;
unsigned long pressStart = 0;
bool held = false;
bool rainbow = false;
bool flashed = false;
bool flashed2 = false;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];
Adafruit_NeoPixel ws2812b(num_pixels, strip, NEO_GRB + NEO_KHZ800);

void flashRed() {
  // Set all pixels to red
  for (int i = 0; i < num_pixels; i++) {
    ws2812b.setPixelColor(i, ws2812b.Color(255, 0, 0));  // Red
  }
  ws2812b.show();

  delay(100);

  for (int j = 0; j < num_pixels; j++) {
    ws2812b.setPixelColor(j, ws2812b.Color(0, 0, 0));  // Clear
  }
  ws2812b.show();

  delay(100);
  
}

void updateLives(int count) {
  ws2812b.clear();  // Clear all first

  for (int i = 0; i < num_pixels; i++) {
    if (i < count) {
      ws2812b.setPixelColor(i, ws2812b.Color(0, 255, 0));  // Green for remaining lives
    } else {
      ws2812b.setPixelColor(i, ws2812b.Color(0, 0, 0));    // Off for lost lives
    }
  }

  ws2812b.show();
}

void flashFirstTwo(int delayTime) {
  for (int j = 0; j < 6; j++) {           // repeat twice
    // Turn first 2 LEDs on
    for (int i = 0; i < 2; i++) {
      ws2812b.setPixelColor(i, ws2812b.Color(0, 255, 0));
    }
    ws2812b.show();
    delay(delayTime);

    // Turn first 2 LEDs off
    for (int i = 0; i < 6; i++) {
      ws2812b.setPixelColor(i, ws2812b.Color(0, 0, 0));
    }
    ws2812b.show();
    delay(delayTime);
  }
}

// Flash only the first LED twice
void flashFirstOne(int delayTime) {
  for (int j = 0; j < 6; j++) {
    // Turn first LED on
    ws2812b.setPixelColor(0, ws2812b.Color(0, 255, 0));
    ws2812b.show();
    delay(delayTime);

    // Turn first LED off
    ws2812b.setPixelColor(0, ws2812b.Color(0, 0, 0));
    ws2812b.show();
    delay(delayTime);
  }
}

void onConnectedController(ControllerPtr ctl) {
  // Only accept if myControllers[0] is free
    if (!myControllers[0]) {
        myControllers[0] = ctl;
        Serial.println("Controller assigned to slot 0");
    } else {
        Serial.println("Ignoring extra controller");
        ctl->disconnect();
    }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
      myControllers[i] = nullptr;
      foundController = true;
      break;
    }
  }

  if (!foundController) {
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void setup() {
  Serial.begin(115200);
  ws2812b.begin();

  for (int k = 0; k < num_pixels; k++) {
    ws2812b.setPixelColor(k, ws2812b.Color(0, 0, 0));  // Clear
  }
  ws2812b.show();

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableNewBluetoothConnections(true);
  BP32.forgetBluetoothKeys();
  leftServo.attach(leftServo_pwm);
  rightServo.attach(rightServo_pwm);
  pinMode(Lmotor1, OUTPUT);
  pinMode(Lmotor2, OUTPUT);
  pinMode(Rmotor1, OUTPUT);
  pinMode(Rmotor2, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  digitalWrite(button, LOW);

  ledcSetup(2, 5000, 8);
  ledcSetup(3, 5000, 8);
  ledcSetup(4, 5000, 8);
  ledcSetup(5, 5000, 8);
  ledcAttachPin(Lmotor1, 2);
  ledcAttachPin(Lmotor2, 3);
  ledcAttachPin(Rmotor1, 4);
  ledcAttachPin(Rmotor2, 5);
}

void MotorA(int LY, int RX) {
  if (LY == 0) { // No input
    ledcWrite(2, 0);
    ledcWrite(3, 0);
  } else if (LY > 0) { // Up
    ledcWrite(2, map(LY, 0, 1024, 400, 525));
    ledcWrite(3, 0);
  } else if (LY < 0) { // Down
    ledcWrite(2, 0);
    ledcWrite(3, map(LY, 0, -1024, 400, 525));
  }
  if (RX < 0) { // Left
    ledcWrite(2, 0);
    ledcWrite(3, map(RX, 0, -1024, 400, 525));
  }
  if (RX > 0) { // Right
    ledcWrite(2, map(RX, 0, 1024, 400, 525));
    ledcWrite(3, 0);
  }

}

void MotorB(int LY, int RX) {
  if (LY == 0) { // No input
    ledcWrite(4, 0);
    ledcWrite(5, 0);
  } else if (LY > 0) { // Up
    ledcWrite(4, map(LY, 0, 1024, 400, 525));
    ledcWrite(5, 0);
  } else if (LY < 0) { // Down
    ledcWrite(4, 0);
    ledcWrite(5, map(LY, 0, -1024, 400, 525));
  }
  if (RX < 0) { // Left
    ledcWrite(4, map(RX, 0, -1024, 400, 525));
    ledcWrite(5, 0);
  }
  if (RX > 0) { // Right
    ledcWrite(4, 0);
    ledcWrite(5, map(RX, 0, 1024, 400, 525));
  }
}

const unsigned long servoActiveTime = 500; // time servo stays in active position
const unsigned long servoReturnTime = 300; // time allowed for servo to return

// Timing and state variables
unsigned long leftStartTime = 0;
bool leftActiveFlag = false;
bool leftReturningFlag = false;
bool leftReady = true;

unsigned long rightStartTime = 0;
bool rightActiveFlag = false;
bool rightReturningFlag = false;
bool rightReady = true;

void updateServos() {
    unsigned long currentTime = millis();

    // ---------------- LEFT SERVO ----------------
    if (!leftActiveFlag && !leftReturningFlag && leftReady && L2 > 0) {
        leftServo.write(90);
        leftStartTime = currentTime;
        leftActiveFlag = true;
        leftReady = false;
    }

    // Check if active time is over -> start returning
    if (leftActiveFlag && (currentTime - leftStartTime >= servoActiveTime)) {
        leftServo.write(0);
        leftActiveFlag = false;
        leftReturningFlag = true;
        leftStartTime = currentTime; // reuse for return timing
    }

    // Check if return time is over -> servo ready again
    if (leftReturningFlag && (currentTime - leftStartTime >= servoReturnTime)) {
        leftReturningFlag = false;
    }

    // Allow new trigger only when fully returned and input released
    if (!leftActiveFlag && !leftReturningFlag && !leftReady && L2 == 0) {
        leftReady = true;
    }

    // ---------------- RIGHT SERVO ----------------
    if (!rightActiveFlag && !rightReturningFlag && rightReady && R2 > 0) {
        rightServo.write(0);
        rightStartTime = currentTime;
        rightActiveFlag = true;
        rightReady = false;
    }

    // Check if active time is over -> start returning
    if (rightActiveFlag && (currentTime - rightStartTime >= servoActiveTime)) {
        rightServo.write(90);
        rightActiveFlag = false;
        rightReturningFlag = true;
        rightStartTime = currentTime; // reuse for return timing
    }

    // Check if return time is over -> servo ready again
    if (rightReturningFlag && (currentTime - rightStartTime >= servoReturnTime)) {
        rightReturningFlag = false;
    }

    // Allow new trigger only when fully returned and input released
    if (!rightActiveFlag && !rightReturningFlag && !rightReady && R2 == 0) {
        rightReady = true;
    }
}

void flashRainbow(int delayTime) {
  const int rainbowColors[5][3] = {
      {255, 0, 0},   // Red
      {255, 165, 0}, // Orange
      {255, 255, 0}, // Yellow
      {0, 255, 0},   // Green
      {0, 0, 255}    // Blue
  };

  for (int i = 0; i < 3; i++) {
    int randomIndex = random(0, 5);  // choose a random colour 0–4
    ws2812b.setPixelColor(i, ws2812b.Color(
      rainbowColors[randomIndex][0],
      rainbowColors[randomIndex][1],
      rainbowColors[randomIndex][2]
    ));
  }

  ws2812b.show();
  delay(delayTime);
}

void loop() {
  bool dataUpdated = BP32.update();
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    ControllerPtr ctl = myControllers[i];
    if (ctl && ctl->isConnected() && ctl->hasData() && ctl->index() == 0) {
      if (ctl->axisRX() == -4) RStickX = 0; else RStickX = map(ctl->axisRX(), 508, -512, 1024, -1024);
      if (ctl->axisY() == -4) LStickY = 0; else LStickY = map(ctl->axisY(), 508, -512, -1024, 1024);
      L2 = ctl->l2();
      R2 = ctl->r2();
      rainbow = (ctl->buttons() & 0x000f) == 0x000f;
    }
  }
  if (lives > 0) {
    updateServos();

  MotorA(LStickY, RStickX);
  MotorB(LStickY, RStickX);

  static unsigned long lastPressTime = 0;  // store the time of last valid press
  const unsigned long cooldown = 2000;     // 2 seconds

  if (digitalRead(button) == HIGH) {
    if (millis() - lastPressTime >= cooldown) {
      lives -= 1;
      lastPressTime = millis();  // reset cooldown timer
    }
  }
  updateLives(lives);
  if (lives == 2 && !flashed2) {
    flashFirstTwo(150);
    flashed2 = true;
  }
  if (lives == 1 && !flashed) {
    flashFirstOne(150);
    flashed = true;
  }
  }
  if (lives == 0) {
    ledcWrite(2, 0);
    ledcWrite(3, 0);
    ledcWrite(4, 0);
    ledcWrite(5, 0);
    leftServo.write(0);
    rightServo.write(90);
    flashRed();
  }
  if (digitalRead(button) == HIGH) { // Check if button pressed for 3s or longer
    if (pressStart == 0) pressStart = millis(); 

    if (lives == 0 && !held && (millis() - pressStart >= 2000)) {
      held = true;
      lives = 4;
      flashed = false;
      flashed2 = false;
    }
  } else {
    pressStart = 0;
    held = false;
  }

  if (rainbow) flashRainbow(150);

  Serial.print("LY: ");
  Serial.print(LStickY);
  Serial.print("\tRX: ");
  Serial.print(RStickX);
  Serial.print("\tL2: ");
  Serial.print(L2);
  Serial.print("\tR2: ");
  Serial.print(R2);
  Serial.print("\tLservo: ");
  Serial.print(Lservo_status);
  Serial.print("\tRservo: ");
  Serial.print(Rservo_status);
  Serial.print("\tButton: ");
  Serial.print(digitalRead(button));
  Serial.print("\tLives: ");
  Serial.println(lives);
}
