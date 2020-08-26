#ifndef CONFIG
#define CONFIG

/**
 * Set this to false to disable Serial logging
 */
#define DEBUG true

/**
 * The name of this device last character is used for the id
 */
#define DEVICE_NAME "ct_power_meter_x"

/**
 * ADC input pin that is used to read out the CT sensors
 */
#define NR_INPUTS 4 // max 10
#define ADC_INPUTS 34, 35, 32, 33

/**
 * Calibration button input
 */
#define CALBIBRATION_BTN 2

/**
 * The voltage of your home, used to calculate the wattage.
 * Try setting this as accurately as possible.
 */
#define HOME_VOLTAGE 230.0

/**
 * The transformer ratio of CT clamp as noted on device.
 * 100 A / 50 mA
 */
#define TRANSFORMER_RATIO 100.0 / 0.05

/**
 * The impedence of the burden resitor used in Ohm.
 */
#define BURDEN_RESISTOR 176

/**
 * WiFi credentials
 */
#define WIFI_NETWORK "****** YOUR WIFI NETWORK NAME *******"
#define WIFI_PASSWORD "****** YOUR WIFI PASSWORD *******"

/**
 * Timeout for the WiFi connection. When this is reached,
 * the ESP goes into deep sleep for 30seconds to try and
 * recover.
 */
#define WIFI_TIMEOUT 20000 // 20 seconds

/**
 * How long should we wait after a failed WiFi connection
 * before trying to set one up again.
 */
#define WIFI_RECOVER_TIME_MS 20000 // 20 seconds


/**
 * Force Emonlib to assume a 3.3V supply to the CT sensor
 */
#define emonTxV3 1


/**
 * Local measurements
 */
#define LOCAL_MEASUREMENTS 30


#define MQTT_CONNECT_DELAY 200
#define MQTT_CONNECT_TIMEOUT 20000 // 20 seconds


/**
 * Syncing time with an NTP server
 */
#define NTP_TIME_SYNC_ENABLED true
#define NTP_SERVER "pool.ntp.org"
#define NTP_OFFSET_SECONDS 3600
#define NTP_UPDATE_INTERVAL_MS 60000

/**
 * Wether or not you want to enable Home Assistant integration
 */
#define HA_ENABLED true
#define HA_ADDRESS "*** YOUR HOME ASSISTANT IP ADDRESSS ***"
#define HA_PORT 1883
#define HA_USER "*** MQTT USER ***"
#define HA_PASSWORD "*** MQTT PASSWORD ***"

// Check which core Arduino is running on. This is done because updating the 
// display only works from the Arduino core.
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#endif