#ifndef TASK_MEASURE_ELECTRICITY
#define TASK_MEASURE_ELECTRICITY

#include <Arduino.h>
#include "EmonLib.h"

#include "../config/config.h"
#include "../config/enums.h"
#include "mqtt-home-assistant.h"

extern DisplayValues gDisplayValues;
extern EnergyMonitor emon1;
extern unsigned short measurements[];
extern unsigned char measureIndex;

void measureElectricity(void * parameter)
{
    for(;;){
      serial_println("[ENERGY] Measuring...");
      long start = millis();

      double amps = emon1.calcIrms(1480);
      double WattsOffset = *static_cast<double*>(parameter);
      double watts = amps * HOME_VOLTAGE;
      watts -= WattsOffset;
      amps = watts / HOME_VOLTAGE;
      serial_print("[ENERGY] Watts: ");
      Serial.println(watts, 4);
      serial_print("[ENERGY] Ampere: ");
      Serial.println(amps, 4);

      gDisplayValues.amps = amps;
      gDisplayValues.watt = watts;

      measurements[measureIndex] = watts;
      measureIndex++;

      if(measureIndex == LOCAL_MEASUREMENTS){
          #if HA_ENABLED == true
            xTaskCreate(
              sendEnergyToHA,
              "HA-MQTT Upload",
              10000,             // Stack size (bytes)
              NULL,             // Parameter
              5,                // Task priority
              NULL              // Task handle
            );
          #endif

          measureIndex = 0;
      }

      long end = millis();

      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took)
      vTaskDelay((1000-(end-start)) / portTICK_PERIOD_MS);
    }    
}

#endif
