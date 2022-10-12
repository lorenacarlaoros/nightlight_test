#include <Wire.h>
#include <TFT_eSPI.h>

#include <Arduino.h>
#include <rpcWiFi.h>
#include <SPI.h>

#include "config.h"

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <iothubtransportmqtt.h>
#include "ntp.h"

IOTHUB_DEVICE_CLIENT_LL_HANDLE _device_ll_handle;

static void connectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *user_context)
{
  if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
  {
    Serial.println("The device client is connected to iothub");
  }
  else
  {
    Serial.println("The device client has been disconnected");
  }
}

int directMethodCallback(const char *method_name, const unsigned char *payload, size_t size, unsigned char **response, size_t *response_size, void *userContextCallback)
{
  Serial.printf("Direct method received %s\r\n", method_name);

  if (strcmp(method_name, "relay_on") == 0)
  {
    digitalWrite(PIN_WIRE_SCL, HIGH);
  }
  else if (strcmp(method_name, "relay_off") == 0)
  {
    digitalWrite(PIN_WIRE_SCL, LOW);
  }

  char resultBuff[16];
  sprintf(resultBuff, "{\"Result\":\"\"}");
  *response_size = strlen(resultBuff);
  *response = (unsigned char *)malloc(*response_size);
  memcpy(*response, resultBuff, *response_size);

  return IOTHUB_CLIENT_OK;
}

void connectIoTHub()
{
  IoTHub_Init();

  _device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(CONNECTION_STRING, MQTT_Protocol);

  if (_device_ll_handle == NULL)
  {
    Serial.println("Failure creating Iothub device. Hint: Check your connection string.");
    return;
  }

  IoTHubDeviceClient_LL_SetConnectionStatusCallback(_device_ll_handle, connectionStatusCallback, NULL);
  IoTHubClient_LL_SetDeviceMethodCallback(_device_ll_handle, directMethodCallback, NULL);
}

void sendTelemetry(const char *telemetry)
{
  IOTHUB_MESSAGE_HANDLE message_handle = IoTHubMessage_CreateFromString(telemetry);
  IoTHubDeviceClient_LL_SendEventAsync(_device_ll_handle, message_handle, NULL, NULL);
  IoTHubMessage_Destroy(message_handle);
}

void connectWiFi()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi..");
    WiFi.begin(SSID, PASSWORD);
    delay(500);
  }

  Serial.println("Connected!");
}

#define DHTPIN 2
#define DHTTYPE DHT11

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

int sensorPin = A0;
int sensorValue = 0;

void setup()
{
  pinMode(A0, INPUT);
  pinMode(D1, OUTPUT);
  pinMode(PIN_WIRE_SCL, OUTPUT);

  Serial.begin(115200);
  Wire.begin();

  tft.begin();
  tft.setRotation(3);
  spr.createSprite(TFT_HEIGHT, TFT_WIDTH);

  connectWiFi();
  initTime();
  connectIoTHub();
  delay(2000);
}

void work_delay(int delay_time)
{
  int current = 0;
  do
  {
    IoTHubDeviceClient_LL_DoWork(_device_ll_handle);
    delay(100);
    current += 100;
  } while (current < delay_time);
}

void loop()
{
  // Display soil moisture
  sensorValue = analogRead(sensorPin);
  sensorValue = map(sensorValue, 560, 450, 0, 100);
  //
  int soil_moisture = analogRead(A0);

  string telemetry;

  Serial.print("Sending telemetry ");
  Serial.println(telemetry.c_str());
  sendTelemetry(telemetry.c_str());

  work_delay(10000);
  //

  String cmdMessage = "Soil Moisture: " + sensorValue;
  Serial.println("Soil Moisture: ");
  Serial.print(sensorValue);
  Serial.print("%");

  int t = 10.0;
  int h = 5.0;

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
    spr.setTextSize(3);
    spr.fillSprite(TFT_RED);
    spr.drawString("The plant is dying!", 10, 100);
    analogWrite(WIO_BUZZER, 150);
    delay(1000);
    analogWrite(WIO_BUZZER, 0);
    digitalWrite(D1, HIGH);
  }
  else
  {
    digitalWrite(D1, LOW);
  }

  spr.pushSprite(0, 0);
  delay(10000);
}
