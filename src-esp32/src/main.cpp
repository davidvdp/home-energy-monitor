#include <Arduino.h>
#include "EmonLib.h"
#include "WiFi.h"
#include <driver/adc.h>
#include "config/config.h"
#include "config/enums.h"
#include <Wire.h>

#include "tasks/fetch-time-from-ntp.h"
#include "tasks/wifi-connection.h"
#include "tasks/wifi-update-signalstrength.h"
#include "tasks/measure-electricity.h"
#include "tasks/mqtt-home-assistant.h"

DisplayValues gDisplayValues;
EnergyMonitor emon1[NR_INPUTS];


// Place to store local measurements before sending them off to AWS
unsigned short pins[] = {ADC_INPUTS};
unsigned short measurements[NR_INPUTS][LOCAL_MEASUREMENTS];
unsigned char measureIndex = 0;
bool calibrating = false;
double WattsOffset[NR_INPUTS];

void calibrate()
{
  if (!calibrating){
    calibrating = true;
    for (size_t i_pin = 0; i_pin < NR_INPUTS; i_pin++)
    {
      double WattsOffsetWork = 0;
      for (short i = 0; i < LOCAL_MEASUREMENTS; i++){
          WattsOffsetWork += measurements[i_pin][i];
          measurements[i_pin][i] = 0;
      }
      WattsOffsetWork /= LOCAL_MEASUREMENTS;
      if (WattsOffset[i_pin] >= 0.001 || WattsOffset[i_pin] <= -0.001){
          serial_print(i_pin);
          serial_println(" has been reset!");
          WattsOffset[i_pin] = 0.0;
      } 
      else {
          WattsOffset[i_pin] = WattsOffsetWork;
          serial_print(i_pin);
          serial_print(" has been calibrated! Watt Offset: ");
          serial_println(WattsOffset[i_pin]);
      } 
    }
  }
}

void setup()
{
  #if DEBUG == true
    Serial.begin(115200);
  #endif 

  for (size_t i_pin = 0; i_pin < NR_INPUTS; i_pin++)
  {
    // Setup the ADC
    analogReadResolution(ADC_BITS);
    pinMode(pins[i_pin], INPUT);
  
    // Initialize emon library
    double current_constant = TRANSFORMER_RATIO / BURDEN_RESISTOR;
    emon1[i_pin].current(pins[i_pin], current_constant);
    WattsOffset[i_pin] = 2.0;
  }


  // Allow for calibration with no-current through CT clamps
  pinMode(CALBIBRATION_BTN, INPUT);
  attachInterrupt(digitalPinToInterrupt(CALBIBRATION_BTN), calibrate, RISING);

  // ----------------------------------------------------------------
  // TASK: Connect to WiFi & keep the connection alive.
  // ----------------------------------------------------------------
  xTaskCreatePinnedToCore(
    keepWiFiAlive,
    "keepWiFiAlive",  // Task name
    5000,            // Stack size (bytes)
    NULL,             // Parameter
    1,                // Task priority
    NULL,             // Task handle
    ARDUINO_RUNNING_CORE
  );

  // ----------------------------------------------------------------
  // Task: measure electricity consumption ;)
  // ----------------------------------------------------------------
  xTaskCreate(
    measureElectricity,
    "Measure electricity",  // Task name
    5000,                  // Stack size (bytes)
    NULL,            // Parameter
    4,                      // Task priority
    NULL                    // Task handle
  );

  // ----------------------------------------------------------------
  // TASK: update time from NTP server.
  // ----------------------------------------------------------------
  #if NTP_TIME_SYNC_ENABLED == true
    xTaskCreate(
      fetchTimeFromNTP,
      "Update NTP time",
      5000,            // Stack size (bytes)
      NULL,             // Parameter
      1,                // Task priority
      NULL              // Task handle
    );
  #endif

  // ----------------------------------------------------------------
  // TASK: update WiFi signal strength
  // ----------------------------------------------------------------
  xTaskCreate(
    updateWiFiSignalStrength,
    "Update WiFi strength",
    1000,             // Stack size (bytes)
    NULL,             // Parameter
    2,                // Task priority
    NULL              // Task handle
  );

  #if HA_ENABLED == true
    xTaskCreate(
      HADiscovery,
      "MQTT-HA Discovery",  // Task name
      5000,                // Stack size (bytes)
      NULL,                 // Parameter
      5,                    // Task priority
      NULL                  // Task handle
    );

    xTaskCreate(
      keepHAConnectionAlive,
      "MQTT-HA Connect",
      5000,
      NULL,
      4,
      NULL
    );
  #endif
}

void loop()
{
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  calibrating = false;
}