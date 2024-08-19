#include <OneWire.h>
#include <DallasTemperature.h>

#include "DHT.h"

#include <TFT_eSPI.h>
#include <SPI.h>

#include "Free_Fonts.h"
#include "upi.h"
#include "fptk.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite upisprite =  TFT_eSprite(&tft);
TFT_eSprite fptksprite =  TFT_eSprite(&tft);
unsigned long drawTime = 0;

#define ONE_WIRE_BUS 15
#define ONE_WIRE_BUS2 22
#define ONE_WIRE_BUS3 0
OneWire oneWire(ONE_WIRE_BUS);
OneWire oneWire2(ONE_WIRE_BUS2);
OneWire oneWire3(ONE_WIRE_BUS3);
DallasTemperature sensors(&oneWire);
DallasTemperature sensors2(&oneWire2);
DallasTemperature sensors3(&oneWire3);

#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define buzzer 27

#define BTN_ADD 33         
#define BTN_SUB 32         
#define BTN_START 25
#define BTN_STOP 14
int minutes = 0;
int prevminutes = 0;
int seconds = 0;
int prevseconds = 0;
bool timerStarted = false;
unsigned long startTime;
bool finished = false;
bool selesai = false;

#define BTN_MODE 19
int i = 0;
bool updateDisplay = true;
int lastValue = -1;    

#define relay1 17
#define relay2 26
#define relay3 21

float tempC = 0;  
float prevTempC = 0;
float tempF = 0;  
float prevTempF = 0;

float tempC2 = 0;  
float prevTempC2 = 0;
float tempF2 = 0;  
float prevTempF2 = 0;

float tempC3 = 0;  
float prevTempC3 = 0;
float tempF3 = 0;  
float prevTempF3 = 0;

float prevh = 0;
float prevt = 0;
float prevhic = 0;

bool buzzerActive = false; 
bool wasAboveThreshold = false;


void setup(void) {
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_WHITE);

  dht.begin();

  sensors.begin();
  sensors2.begin();
  sensors3.begin();

  upisprite.createSprite(31,31);
  upisprite.setSwapBytes(true);
  upisprite.pushImage(0,0,31,31,upi);
  upisprite.pushSprite(440,280,TFT_PURPLE);

  fptksprite.createSprite(74,31);
  fptksprite.setSwapBytes(true);
  fptksprite.pushImage(0,0,74,31,logofptk2022);
  fptksprite.pushSprite(350,280,TFT_BLACK);

  pinMode(BTN_ADD, INPUT_PULLUP);
  pinMode(BTN_SUB, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_STOP, INPUT_PULLUP);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);

  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
}

void loop() {
  if (digitalRead(BTN_MODE) == LOW) {
    delay(400);
    i++;
    if (i > 4) {
      i = 0;
    }
    updateDisplay = true;
    digitalWrite(buzzer,HIGH);
    delay(250);
    digitalWrite(buzzer,LOW);
    while (digitalRead(BTN_MODE) == LOW);
    Serial.println(i);  // Print the current value of i
  }

  /////////////////////////////////////////////////////////////////
  float t = dht.readTemperature();
  if (updateDisplay) {
    if (i != lastValue) {
      tft.fillRect(0, 0, 480, 279, TFT_WHITE);  // Clear the display
      lastValue = i;  // Update the lastValue to the current value of i
    }
  }
  
  if (i == 0) {
    timer();
  } else if (i == 1) {
    chamber();
  } else if (i == 2) {
    kondenser();
  } else if (i == 3) {
    kompressor();
  } else if (i == 4) {
    evaporator();
  }

  
  if (timerStarted) {
    unsigned long elapsedTime = (millis() - startTime) / 1000;
    int remainingMinutes = minutes - elapsedTime / 60;
    int remainingSeconds = 59 - (elapsedTime % 60);

    if (remainingMinutes < 0) {
      timerStarted = false; // Timer finished
      remainingMinutes = 0;
      remainingSeconds = 0;
      finished = true;
      selesai = true;
      digitalWrite(relay1, HIGH);
    }
    if (digitalRead(BTN_START) == LOW) {
      delay(50);  // Debounce delay
      selesai = false;
      while (digitalRead(BTN_START) == LOW);
    }
    if (i != 0) {
      displayTime2(remainingMinutes, remainingSeconds);
    } else {}
    if (digitalRead(BTN_STOP) == LOW) {
      delay(200);  // Debounce delay
      timerStarted = false; // Timer finished
      remainingMinutes = 0;
      remainingSeconds = 0;
      minutes = 0;
      finished = true;
      selesai = true;
      digitalWrite(relay1, HIGH);
    }
  }
  tft.setTextColor(TFT_NAVY);
  tft.setTextSize(1);
  tft.setFreeFont(FS9);
  tft.setCursor(290, 30); 
  tft.println("R1");
  tft.setCursor(380, 30);
  tft.print("R2");
  if (t < 29) {
    digitalWrite(relay3, LOW);
    digitalWrite(relay2, HIGH);
    tft.fillCircle(330, 24, 10, TFT_RED);
    tft.fillCircle(420, 24, 10, TFT_GREEN);
    if (!buzzerActive && !wasAboveThreshold) {
      digitalWrite(buzzer, HIGH);
      delay(125);
      digitalWrite(buzzer, LOW);
      delay(125);
      digitalWrite(buzzer, HIGH);
      delay(125);
      digitalWrite(buzzer, LOW);
      delay(125);
      buzzerActive = true;
    }
  wasAboveThreshold = true;
  } else {
    digitalWrite(relay2, LOW);
    digitalWrite(relay3, HIGH);
    tft.fillCircle(330, 24, 10, TFT_GREEN);
    tft.fillCircle(420, 24, 10, TFT_RED);
    if (wasAboveThreshold) {
      digitalWrite(buzzer, HIGH);
      delay(125);
      digitalWrite(buzzer, LOW);
      delay(125);
      digitalWrite(buzzer, HIGH);
      delay(125);
      digitalWrite(buzzer, LOW);
      delay(125);
      wasAboveThreshold = false; 
      buzzerActive = false;
    }
  }
  /////////////////////////////////////////////////////////////////

  if (finished) {
    finish();
  }
}

void timer() {
  if (!timerStarted) {
  // Adjust the timer
    if (digitalRead(BTN_ADD) == LOW) {
      if (minutes < 91) {
        delay(200);  // Debounce delay
        minutes += 5;
        while (digitalRead(BTN_ADD) == LOW);
      } else if (minutes > 94 && minutes <= 98) {
        delay(200);
        minutes++;
        while (digitalRead(BTN_ADD) == LOW);
      } else if (minutes > 98) {
        digitalWrite(buzzer, HIGH);
        delay(250);
        digitalWrite(buzzer, LOW);
        while (digitalRead(BTN_ADD) == LOW);
      }
    }
    
    if (digitalRead(BTN_SUB) == LOW) {
      delay(200);  // Debounce delay
      if (minutes > 0) {
        minutes--;
      }
      while (digitalRead(BTN_SUB) == LOW);
    }

    // Start the timer
    if (digitalRead(BTN_START) == LOW) {
      delay(200);  // Debounce delay
      if (minutes > 0) {
        timerStarted = true;
        startTime = millis();
        minutes--;
      }
      digitalWrite(relay1, LOW);
      while (digitalRead(BTN_START) == LOW);
    }

    // Display the time
    displayTime(minutes, 0);
  } else {
    // Timer countdown
    unsigned long elapsedTime = (millis() - startTime) / 1000;
    int remainingMinutes = minutes - elapsedTime / 60;
    int remainingSeconds = 59 - (elapsedTime % 60);

    if (remainingMinutes < 0) {
      timerStarted = false; // Timer finished
      remainingMinutes = 0;
      remainingSeconds = 0;
      finished = true;
      selesai = true;
      digitalWrite(relay1, HIGH);
    }
    displayTime(remainingMinutes, remainingSeconds);
  }
}



void displayTime(int minutes, int seconds) {
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(200, 30);
  tft.setFreeFont(FMB12);
  tft.setTextSize(1);
  tft.print("Timer");

  if (minutes < 10) {
    tft.setTextColor(TFT_BLUE);
    tft.setCursor(190,170);
    tft.setFreeFont(FMB12);
    tft.setTextSize(2);
    tft.print(minutes);
  } else {
    tft.setTextColor(TFT_BLUE);
    tft.setCursor(162,170);
    tft.setFreeFont(FMB12);
    tft.setTextSize(2);
    tft.print(minutes);
  }
  tft.print(":");
  if (seconds < 10) {
    tft.print("0");
  }
  tft.print(seconds);

  if (seconds != prevseconds) {
    tft.fillRect(240, 135, 65, 40, TFT_WHITE); 
    prevseconds = seconds;
  }
  if (minutes != prevminutes) {
    tft.fillRect(160, 135, 60, 40, TFT_WHITE); 
    prevminutes = minutes;
  }
}

void displayTime2(int minutes, int seconds) {
  if (minutes < 10) {
    tft.setTextColor(TFT_BLUE);
    tft.setCursor(380,270);
    tft.setFreeFont(FMB9);
    tft.setTextSize(2);
    tft.print(minutes);
  } else { 
    tft.setTextColor(TFT_BLUE);
    tft.setCursor(352,270);
    tft.setFreeFont(FMB9);
    tft.setTextSize(2);
    tft.print(minutes);
  }
  tft.print(":");
  if (seconds < 10) {
    tft.print("0");
  }
  tft.print(seconds);

  if (seconds != prevseconds) {
    tft.fillRect(425, 235, 65, 40, TFT_WHITE); 
    prevseconds = seconds;
  }
  if (minutes != prevminutes) {
    tft.fillRect(365, 235, 65, 40, TFT_WHITE); 
    prevminutes = minutes;
  }
}

void kompressor() {
  sensors2.requestTemperatures();
  tempC2 = sensors2.getTempCByIndex(0);
  tempF2 = sensors2.getTempFByIndex(0);

  float t = dht.readTemperature();

  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(FMB12);
  tft.setTextSize(1);
  
  tft.setCursor(20, 30); 
  tft.println("Compressor");
  tft.setCursor(20, 50);
  tft.print("Temperature");

  tft.setFreeFont(FM24);
  tft.setTextColor(TFT_RED);
  tft.setCursor(140, 130);
  tft.setTextSize(1);
  tft.print(tempC2);
  tft.print(" C");
  if (tempC2 != prevTempC2) {
    tft.fillRect(135, 90, 210, 60, TFT_BLACK); 
    prevTempC2 = tempC2;
  }

  tft.setFreeFont(FM24);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(140, 200);
  tft.setTextSize(1);
  tft.print(tempF2);
  tft.print(" F");
  if (tempF2 != prevTempF2) {
    tft.fillRect(135, 160, 210, 60, TFT_BLACK); 
    prevTempF2 = tempF2;
  }
}

void kondenser() {
  sensors.requestTemperatures();
  tempC = sensors.getTempCByIndex(0);
  tempF = sensors.getTempFByIndex(0);

  float t = dht.readTemperature();

  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(FMB12);
  tft.setTextSize(1);
  
  tft.setCursor(20, 30); 
  tft.println("Condenser");
  tft.setCursor(20, 50);
  tft.print("Temperature");

  tft.setFreeFont(FM24);
  tft.setTextColor(TFT_RED);
  tft.setCursor(140, 130);
  tft.setTextSize(1);
  tft.print(tempC);
  tft.print(" C");
  if (tempC != prevTempC) {
    tft.fillRect(135, 90, 210, 60, TFT_BLACK); 
    prevTempC = tempC;
  }

  tft.setFreeFont(FM24);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(140, 200);
  tft.setTextSize(1);
  tft.print(tempF);
  tft.print(" F");
  if (tempF != prevTempF) {
    tft.fillRect(135, 160, 210, 60, TFT_BLACK); 
    prevTempF = tempF;
  }
}

void evaporator() {
  sensors3.requestTemperatures();
  tempC3 = sensors3.getTempCByIndex(0);
  tempF3 = sensors3.getTempFByIndex(0);

  float t = dht.readTemperature();

  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(FMB12);
  tft.setTextSize(1);
  
  tft.setCursor(20, 30); 
  tft.println("Evaporator");
  tft.setCursor(20, 50);
  tft.print("Temperature");

  tft.setFreeFont(FM24);
  tft.setTextColor(TFT_RED);
  tft.setCursor(140, 130);
  tft.setTextSize(1);
  tft.print(tempC3);
  tft.print(" C");
  if (tempC3 != prevTempC3) {
    tft.fillRect(135, 90, 210, 60, TFT_BLACK); 
    prevTempC3 = tempC3;
  }

  tft.setFreeFont(FM24);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(140, 200);
  tft.setTextSize(1);
  tft.print(tempF3);
  tft.print(" F");
  if (tempF3 != prevTempF3) {
    tft.fillRect(135, 160, 210, 60, TFT_BLACK); 
    prevTempF3 = tempF3;
  }
}

void chamber() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float hic = dht.computeHeatIndex(t, h, false);
  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(FMB18);
  tft.setTextSize(1);
  tft.setCursor(27, 32); 
  tft.println("Chamber");

  tft.setFreeFont(FM24);
  tft.setTextColor(TFT_RED);
  tft.setCursor(27, 100);
  tft.setTextSize(1);
  tft.print(t);
  tft.print(" C");
  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(FMB9);
  tft.setTextSize(1);
  tft.setCursor(250, 90); 
  tft.println("Temperature");
  if (t != prevt) {
    tft.fillRect(22, 58, 210, 60, TFT_BLACK); 
    prevt = t;
  }

  tft.setFreeFont(FM24);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(27, 170);
  tft.setTextSize(1);
  tft.print(h);
  tft.print(" %");
  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(FMB9);
  tft.setTextSize(1);
  tft.setCursor(250, 160); 
  tft.println("Humidity");
  if (h != prevh) {
    tft.fillRect(22, 128, 210, 60, TFT_BLACK); 
    prevh = h;
  }

  tft.setFreeFont(FM24);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(27, 240);
  tft.setTextSize(1);
  tft.print(hic);
  tft.print(" C");
  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(FMB9);
  tft.setTextSize(1);
  tft.setCursor(250, 230); 
  tft.println("Heat Index");
  if (hic != prevhic) {
    tft.fillRect(22, 198, 210, 60, TFT_BLACK);
    prevhic = hic;
  }
}

void finish() {
  if (selesai) {
    digitalWrite(buzzer,HIGH);
    delay(62.5);
    digitalWrite(buzzer,LOW);
    delay(62.5);
    digitalWrite(buzzer,HIGH);
    delay(62.5);
    digitalWrite(buzzer,LOW);
    delay(62.5);
    digitalWrite(buzzer,HIGH);
    delay(62.5);
    digitalWrite(buzzer,LOW);
    delay(62.5);
    digitalWrite(buzzer,HIGH);
    delay(62.5);
    digitalWrite(buzzer,LOW);
    delay(62.5);
    delay(500);
    if (digitalRead(BTN_START) == LOW) {
      delay(50);  // Debounce delay
      selesai = false;
      while (digitalRead(BTN_START) == LOW);
    }
  } else {
    digitalWrite(buzzer,LOW);
  }
}