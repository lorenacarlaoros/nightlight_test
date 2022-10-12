#include <Wire.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

int sensorValue = 0;

void setup()
{
  pinMode(A0, INPUT);
  pinMode(WIO_BUZZER, OUTPUT);
  pinMode(D1, OUTPUT);

  Serial.begin(9600);
  Wire.begin();

  tft.begin();
  tft.setRotation(3);
  spr.createSprite(TFT_HEIGHT, TFT_WIDTH);
}

void loop()
{
  // Display soil moisture
  sensorValue = analogRead(A0);
  sensorValue = map(sensorValue, 560, 450, 0, 100);

  String cmdMessage = "Soil Moisture: " + sensorValue;
  Serial.println("Soil Moisture: ");
  Serial.print(sensorValue);
  Serial.print("%");

  int t = 10.0;
  int h = 5.0;

  // Design WIO display
  spr.fillSprite(TFT_WHITE);
  spr.fillRect(0, 0, 320, 50, TFT_DARKGREEN);
  spr.setTextColor(TFT_WHITE);
  spr.setTextSize(3);
  spr.drawString("Smart Garden", 50, 15);

  spr.drawFastVLine(150, 50, 190, TFT_DARKGREEN);
  spr.drawFastHLine(0, 140, 320, TFT_DARKGREEN);

  // Display temperature
  spr.setTextColor(TFT_BLACK);
  spr.setTextSize(2);
  spr.drawString("Temperature", 10, 65);
  spr.setTextSize(3);
  spr.drawNumber(t, 50, 95);
  spr.drawString("C", 90, 95);

  // Display humidity
  spr.setTextSize(2);
  spr.drawString("Hunidity", 25, 160);
  spr.setTextSize(3);
  spr.drawNumber(h, 30, 190);
  spr.drawString("%RH", 70, 190);

  spr.setTextSize(2);
  spr.drawString("Soil Moisture", 160, 65);
  spr.setTextSize(3);
  spr.drawNumber(sensorValue, 200, 95);
  spr.drawString("%", 260, 95);

  if (sensorValue < 20)
  {
    spr.setTextSize(2);
    spr.fillSprite(TFT_RED);
    spr.drawString("The plant is dying!", 10, 100);
    analogWrite(WIO_BUZZER, 150);
    delay(1000);
    analogWrite(WIO_BUZZER, 0);
    delay(1000);
  }

  spr.pushSprite(0, 0); // Push to LCD

  // Water pump
  digitalWrite(D1, HIGH);
  delay(10000);
  digitalWrite(D1, LOW);
  delay(1000);
}