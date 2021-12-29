/*
  mom-timer
  by zach fredin
  12-23-2015

  my mom needed a kitchen timer, and i needed to get a christmas present for her. this
  timer is a bit overpowered, but maybe a future firmware update will increase its
  functionality. it's based on a Teensy 3.2 board, and includes two pushbutton switches,
  three LEDs, a potentiometer, a speaker, and an Adafruit SSD1306 32x128 I2C OLED display.

  MIT license.
*/

#include <SPI.h>
#include <Wire.h>
#include <Bounce.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 2
#define BUTTON_GREEN 12 //note that, uh, green is low when pressed
#define BUTTON_RED 0
#define SPEAKER 7
#define KNOB A2
#define LED_BLUE 4
#define LED_RED 5
#define LED_YELLOW 6

// set up OLED display and button debouncers
Adafruit_SSD1306 display(OLED_RESET);
Bounce buttonGreen = Bounce(BUTTON_GREEN, 10);
Bounce buttonRed = Bounce(BUTTON_RED, 10);

// input variables
int potState = 0;
int potStateScaled = 0; // scales input to 0-59
int buttonStateGreen = 0;
int buttonStateGreenPrev = 0;
int buttonStateRed = 0;
int buttonStateRedPrev = 0;

// timer variables
int seconds = 0;
int minutes = 0;
long previousMillis = 0;
int LEDflashstate = 0;



/* state variables
    MODE = 0: setup_seconds
    MODE = 1: setup_minutes
    MODE = 2: setup_confirm
    MODE = 3: run
    MODe = 4: pause
    MODe = 5: alarm
*/
int MODE = 0;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0X3C);
  pinMode(BUTTON_GREEN, INPUT);
  pinMode(BUTTON_RED, INPUT);
  pinMode(KNOB, INPUT);
  pinMode(SPEAKER, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);

  randomSeed(500); // same alarm each time!

  showIntro();
  MODE = 0;
}

void loop() {
  unsigned long currentMillis = millis();
  getInputState();
  if(MODE == 0) { // setup_minutes
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    minutes = scaleInput(potState);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("set minutes");
    display.setTextSize(2);
    display.print(minutes);
    display.println(":__");
    display.setTextSize(1);
    display.println("green to confirm");
    display.display();
    if ((buttonStateGreen == LOW) & (buttonStateGreenPrev == HIGH)) {
      MODE += 1;
      getInputState();
    }
  }

  if(MODE == 1) { // setup_seconds
    seconds = scaleInput(potState);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("set seconds");
    display.setTextSize(2);
    display.print(minutes);
    display.print(":");
    display.println(seconds);
    display.setTextSize(1);
    display.println("green to confirm");
    display.display();
    if ((buttonStateGreen == LOW) & (buttonStateGreenPrev == HIGH)) {
      MODE += 1;
      getInputState();
    }
  }

  if(MODE == 2) { // setup_confirm
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("timer ready:");
    display.setTextSize(2);
    if (minutes < 10) {
      display.print("0");
    }
    display.print(minutes);
    display.print(":");
    if (seconds < 10) {
      display.print("0");
    }
    display.println(seconds);
    display.setTextSize(1);
    display.println("green to start!");
    display.display();
    if ((buttonStateGreen == LOW) & (buttonStateGreenPrev == HIGH)) {
      MODE += 1;
      getInputState();
    }
  }

  if(MODE == 3) { // run
    if((buttonStateGreen == LOW) & (buttonStateGreenPrev == HIGH)) {
      MODE += 1;
      getInputState();
    }

    display.clearDisplay();
    display.setTextSize(4);
    display.setCursor(0,0);
    if (minutes < 10) {
      display.print("0");
    }
    display.print(minutes);
    display.print(":");
    if (seconds < 10) {
      display.print("0");
    }
    display.println(seconds);
    display.display();
    if(currentMillis - previousMillis > 1000) { // "calibrated"...
      previousMillis = currentMillis;
      if (seconds % 2 == 0) {
        digitalWrite(LED_YELLOW, HIGH);
      }
      else {
        digitalWrite(LED_YELLOW, LOW);
      }
      if (seconds == 0) {
        minutes -= 1;
        seconds = 60;
      }
      seconds -= 1;

    }
    if((seconds == 0) & (minutes == 0)) {
      MODE = 5;
    }
  }

  if(MODE == 4) { // pause
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    if (minutes < 10) {
      display.print("0");
    }
    display.print(minutes);
    display.print(":");
    if (seconds < 10) {
      display.print("0");
    }
    display.println(seconds);
    display.setTextSize(1);
    display.println("timer is paused!");
    display.println("green to unpause.");
    display.display();
    if((buttonStateGreen == LOW) & (buttonStateGreenPrev == HIGH)) {
      MODE -= 1;
      getInputState();

    }
    if((buttonStateRed == HIGH) & (buttonStateRedPrev == LOW)) {
      display.clearDisplay();
      display.display();
      delay(1000);
      MODE = 0;
      getInputState();
    }
  }

  else if(MODE == 5) { //Alarm mode
    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(0,0);
    display.println("ALARM!");
    display.setTextSize(1);
    display.println("green to reset.");
    display.display();
    if(currentMillis - previousMillis > 200) {
      previousMillis = currentMillis;
      if (LEDflashstate == 0) {
        digitalWrite(LED_BLUE, HIGH);
        digitalWrite(LED_RED, LOW);
        LEDflashstate = 1;
        tone(SPEAKER, random(200, 600), 200);
      }
      else {
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_RED, HIGH);
        LEDflashstate = 0;
        tone(SPEAKER, random(600, 1800), 200);
      }
    }
    if((buttonStateGreen == LOW) & (buttonStateRedPrev == HIGH)) {
      display.clearDisplay();
      display.display();
      delay(1000);
      MODE = 0;
      getInputState();
    }
  }

}

int scaleInput(int rawValue) {
  // okay, so this cuts out ~10% of the pot's range.. oh well, bitshifts are the best.
  if(rawValue < 960) {
    return rawValue >> 4;
  }
  else {
    return 59;
  }
}

void getInputState() {
  // store previous button values
  buttonStateGreenPrev = buttonStateGreen;
  buttonStateRedPrev = buttonStateRed;

  // update button and knob values
  buttonGreen.update();
  buttonRed.update();
  potState = analogRead(KNOB);
  buttonStateGreen = buttonGreen.read();
  buttonStateRed = buttonRed.read();
}

void showIntro() {
  // splash screen and operating instructions (includes delays)

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("mom-timer");
  display.setTextSize(1);
  display.println("firmware v0.1");
  display.println("merry xmas, mom");
  display.display();
  tone(SPEAKER, 440, 100);
  digitalWrite(LED_BLUE, HIGH);
  delay(3000);
  display.clearDisplay();
  digitalWrite(LED_BLUE, LOW);

  display.setCursor(0,0);
  display.println("TIMER SETUP MODE");
  display.println("set time with knob.");
  display.println("green to confirm");
  display.println("and start countdown.");
  display.display();
  tone(SPEAKER, 440, 100);
  digitalWrite(LED_YELLOW, HIGH);
  delay(3000);
  display.clearDisplay();
  digitalWrite(LED_YELLOW, LOW);

  display.setCursor(0,0);
  display.println("TIMER RUN MODE");
  display.println("green button pauses");
  display.println("and resets. red");
  display.println("button was miswired.");
  display.display();
  tone(SPEAKER, 440, 100);
  digitalWrite(LED_RED, HIGH);
  delay(3000);
  display.clearDisplay();
  digitalWrite(LED_RED, LOW);
}
