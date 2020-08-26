#ifndef TASK_HOME_ASSISTANT
#define TASK_HOME_ASSISTANT

#if HA_ENABLED == true

    #include <Arduino.h>
    #include <WiFiClient.h>
    #include <MQTTClient.h>
    #include "../config/config.h"
    #include <pgmspace.h>

    WiFiClient HA_net;
    MQTTClient HA_mqtt(1024);

    extern double measurements[][LOCAL_MEASUREMENTS];
    extern double WattsOffset[NR_INPUTS];

    const char* PROGMEM HA_discovery_msg = "{"
            "\"name\":\"" DEVICE_NAME "\","
            "\"device_class\":\"power\","
            "\"unit_of_measurement\":\"W\","
            "\"icon\":\"mdi:transmission-tower\","
            "\"state_topic\":\"homeassistant/sensor/" DEVICE_NAME "/state\","
            "\"value_template\":\"{{ value_json.power}}\","
            "\"device\": {"
                "\"name\":\"" DEVICE_NAME "\","
                "\"sw_version\":\"2.0\","
                "\"model\":\"HW V2\","
                "\"manufacturer\":\"Unknown\","
                "\"identifiers\":[\"" DEVICE_NAME "\"]"
            "}"
        "}";

    // const char PROGMEM HA_discovery_msg[1024];

    /**
     * Established a connection to Home Assistant MQTT broker.
     * 
     * This task should run continously. It will check if an
     * MQTT connection is active and if so, will sleep for 1
     * minute. If not, a new connection will be established.
     */
    void keepHAConnectionAlive(void * parameter){
        for(;;){
            // When we are connected, loop the MQTT client and sleep for 0,5s
            if(HA_mqtt.connected()){
                HA_mqtt.loop();
                vTaskDelay(250 / portTICK_PERIOD_MS);
                continue;
            }

            if(!WiFi.isConnected()){
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                continue;
            }

            serial_println(F("[MQTT] Connecting to HA..."));
            HA_mqtt.begin(HA_ADDRESS, HA_PORT, HA_net);

            long startAttemptTime = millis();
        
            while (!HA_mqtt.connect(DEVICE_NAME, HA_USER, HA_PASSWORD) &&
                    millis() - startAttemptTime < MQTT_CONNECT_TIMEOUT)
            {
                vTaskDelay(MQTT_CONNECT_DELAY / portTICK_PERIOD_MS);
            }

            if(!HA_mqtt.connected()){
                serial_println(F("[MQTT] HA connection failed. Waiting 30s.."));
                vTaskDelay(30000 / portTICK_PERIOD_MS);
                continue;
            }

            serial_println(F("[MQTT] HA Connected!"));
        }
    }

    void get_device_name(int i_device, char * device_name){
        char device_id[2] = "\0";
        sprintf(device_id, "%d", i_device);
        strcpy(&device_name[strlen(device_name)-1], device_id);
    }

    void get_discovery_msg(int i_device, char * HA_discovery_msg, char * node)
    {
        strcpy(HA_discovery_msg, "{"
                "\"name\":\"" DEVICE_NAME "\","
                "\"device_class\":\"power\","
                "\"unit_of_measurement\":\"W\","
                "\"icon\":\"mdi:transmission-tower\","
                "\"state_topic\":\"homeassistant/sensor/" DEVICE_NAME "/state\","
                "\"value_template\":\"{{ value_json.power}}\","
                "\"device\": {"
                    "\"name\":\"" DEVICE_NAME "\","
                    "\"sw_version\":\"2.0\","
                    "\"model\":\"HW V2\","
                    "\"manufacturer\":\"Unknown\","
                    "\"identifiers\":[\"" DEVICE_NAME "\"]"
                "}}");

        char device_name[] = DEVICE_NAME;
        get_device_name(i_device, device_name);
        strcpy(node, "homeassistant/sensor/" DEVICE_NAME "/config");
        for (;;){
            char * p = strstr(HA_discovery_msg, DEVICE_NAME);
            if (!p){break;}
            memcpy(p, device_name, strlen(device_name));
        }
        char * p2 = strstr(node, DEVICE_NAME);
        memcpy(p2, device_name, strlen(device_name));
    }

    /**
     * TASK: Every 15 minutes we send Home Assistant a discovery message
     *       so that the energy monitor shows up in the device registry.
     */
    void HADiscovery(void * parameter){
        for(;;){
            if(!HA_mqtt.connected()){
                serial_println("[MQTT] HA: no MQTT connection.");
                vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
                continue;
            }

            serial_println("[MQTT] HA sending auto discovery");
            for (size_t i_pin = 0; i_pin < NR_INPUTS; i_pin++)
            {
                char PROGMEM HA_discovery_msg[1024];
                char PROGMEM node[256];
                get_discovery_msg(i_pin, HA_discovery_msg, node);
                HA_mqtt.publish(node, HA_discovery_msg);
                serial_println(HA_discovery_msg);
            }
            vTaskDelay(15 * 60 * 1000 / portTICK_PERIOD_MS);
        }
    }

    void sendEnergyToHA(void * parameter){
        if(!HA_mqtt.connected()){
        serial_println("[MQTT] Can't send to HA without MQTT. Abort.");
        vTaskDelete(NULL);
        }

        for (size_t i_input = 0; i_input < NR_INPUTS; i_input++)
        {
            char msg[30];
            double power = 0;
            for (size_t i_meas = 0; i_meas < LOCAL_MEASUREMENTS; i_meas++)
            {
                power += measurements[i_input][i_meas];
            }
            power /= LOCAL_MEASUREMENTS;
            power -= WattsOffset[i_input];
            
            strcpy(msg, "{\"power\":");
            strcat(msg, String(power).c_str());
            strcat(msg, "}");
            serial_print("[MQTT] HA publish: ");
            serial_println(msg);
            char node[256];
            strcpy(node, "homeassistant/sensor/" DEVICE_NAME "/state");
            char * p2 = strstr(node, DEVICE_NAME);
            char device_name[] = DEVICE_NAME;
            get_device_name(i_input, device_name);
            memcpy(p2, device_name, strlen(device_name));
            HA_mqtt.publish(node, msg);
        }  

        // Task is done!
        vTaskDelete(NULL);
    }
#endif
#endif
