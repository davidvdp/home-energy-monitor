#ifndef TASK_MEASURE_ELECTRICITY
#define TASK_MEASURE_ELECTRICITY

#include <Arduino.h>
#include "EmonLib.h"

#include "../config/config.h"
#include "../config/enums.h"
#include "mqtt-home-assistant.h"

extern DisplayValues gDisplayValues;
extern EnergyMonitor emon1[];
extern unsigned short measurements[NR_INPUTS][LOCAL_MEASUREMENTS];
extern unsigned char measureIndex;
extern double WattsOffset[NR_INPUTS];

void measureElectricity(void * parameter)
{
    for(;;){
      serial_println("[ENERGY] Measuring...");
      serial_println("[ENERGY] \tWatts\tAmpere");
       long start = millis();
      for (size_t i_pin = 0; i_pin < NR_INPUTS; i_pin++)
      {    
        double amps = emon1[i_pin].calcIrms(1480);
        double watts = amps * HOME_VOLTAGE;
        watts -= WattsOffset[i_pin];
        amps = watts / HOME_VOLTAGE;
        serial_print("[ENERGY] ");
        serial_print(i_pin);
        serial_print("\t");
        Serial.print(watts, 2);
        serial_print("\t");
        Serial.println(amps, 4);

        gDisplayValues.amps = amps;
        gDisplayValues.watt = watts;

        measurements[i_pin][measureIndex] = watts;    
      }
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
