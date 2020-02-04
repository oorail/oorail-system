/*
 *    oorail-ddc-track-module
 *    version 1.0.0
 *    
 *    Copyright (c) 2020 IdeaPier LLC, All Rights Reserved
 * 
 */

/*
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *   
 */

/*
 *    For more information about this program visit:
 *    
 *      https://oorail.co.uk
 *      https://youtube.com/oorail
 *      
 */

/*
 * Supported Hardware:
 * ====================
 * 
 * Microcontrollers:
 * ------------------
 * 
 *  - ESP32/ESP32S Family of Modules
 *  
 * Motor Drivers: 
 * ---------------
 * 
 *  - L298N modules
 *  
 */

/*
 * Feature Flags:
 * 
 * Optional features - enabling will use more ESP32 resources
 * 
 * OORAIL_FEATURE_WIFI_LED  - Defining this enables the WiFi LED to turn on/off when processing web traffic
 * OORAIL_FEATURE_ASYNC_WEB - Asynchronous HTTP Server Support - NOT IMPLEMENTED YET
 * 
 */
 
/*
#define OORAIL_FEATURE_WIFI_LED 1
#define OORAIL_FEATURE_ASYNC_WEB 1
*/

#include <WiFi.h>
#include <WiFiClient.h>
#if !defined(OORAIL_FEATURE_ASYNC_WEB)
#include <WebServer.h>
#else
#include <ESPAsyncWebServer.h>
#endif
#include <ESPmDNS.h>
#include <strings.h>

#define OORAIL_PROJECT "oorail-track-module"
#define OORAIL_VERSION "1.0.0"
#define OORAIL_COPYRIGHT "Copyright (c) 2020 IdeaPier LLC (oorail.co.uk)"
#define OORAIL_LICENSE "GPLv3"
#define OORAIL_PORT 80

#define OORAIL_TRACK_A 0
#define OORAIL_TRACK_B 1

#define OORAIL_BRAKE_OFF 0
#define OORAIL_BRAKE_ON 1 

#define OORAIL_FALSE 0
#define OORAIL_TRUE 1

#define OORAIL_TRACK_ERROR 0xFE
#define OORAIL_TRACK_FORWARD 0xF0
#define OORAIL_TRACK_REVERSE 0x0F

#define OORAIL_TRACK_MODULE_OUTPUT 2

/*
 * PWM Frequency
 * ==============
 * 
 * The MIN and MAX frequencies below are set at 2kHz and 40kHz respectively.
 * The default is 28kHz (28000), The values below are designed to prevent the user
 * experimenting with this module from doing something bad to their loco motors.
 * 
 * Note: This version of oorail-ddc-tm only supports L298N which has a maximum output of 40kHz
 *       Will add support for better motor drivers in upcoming releases
 * 
 */
#define OORAIL_PWM_FREQ_MIN 2000                  /* CHANGE THIS ONLY IF YOU KNOW WHAT YOU ARE DOING */
#define OORAIL_PWM_FREQ_MAX 40000                 /* DO NOT CHANGE THIS */

#define OORAIL_PWM_FREQ_DEFAULT 28000
#define OORAIL_PWM_FREQ_SAFE 28000                /* DO NOT CHANGE THIS */

#define OORAIL_PWM_RES_DEFAULT 10                 /* Default to 10-bit resolution */
#define OORAIL_PWM_CHAN_A 0                       /* Default PWM Channel for Track A */
#define OORAIL_PWM_CHAN_B 1                       /* Default PWM Channel for Track B */

/* 
 * This is setup for left hand running.
 * 
 * Assumes Track A is outer - forward
 * Assumes Track B is inner - reverse
 * 
 * Assumes both Track A and B are wired with the same polarity
 * 
 */
 
#define OORAIL_TRACK_FORWARD_IN1 HIGH
#define OORAIL_TRACK_FORWARD_IN2 LOW
#define OORAIL_TRACK_REVERSE_IN1 LOW
#define OORAIL_TRACK_REVERSE_IN2 HIGH


/* Manufacturers */
#define OORAIL_MANUFACTURER_ACCURASCALE   0xA0
#define OORAIL_MANUFACTURER_BACHMANN      0xA1
#define OORAIL_MANUFACTURER_CAVALEX       0xA2
#define OORAIL_MANUFACTURER_DAPOL         0xA3
#define OORAIL_MANUFACTURER_DJM           0xA4
#define OORAIL_MANUFACTURER_HATTONS       0xA5
#define OORAIL_MANUFACTURER_HELJAN        0xA6
#define OORAIL_MANUFACTURER_HORNBY        0xA7
#define OORAIL_MANUFACTURER_KATO          0xA8
#define OORAIL_MANUFACTURER_KERNOW        0xA9
#define OORAIL_MANUFACTURER_LIMA          0xAA
#define OORAIL_MANUFACTURER_MURPHYMODELS  0xAB
#define OORAIL_MANUFACTURER_OXFORDRAIL    0xAC
#define OORAIL_MANUFACTURER_RAILS         0xAD
#define OORAIL_MANUFACTURER_RAPIDO        0xAE
#define OORAIL_MANUFACTURER_REALTRACK     0xAF
#define OORAIL_MANUFACTURER_SCRATCHBUILT  0xB0
#define OORAIL_MANUFACTURER_TRACKSIDE3D   0xB1

/* Loco Types */
#define OORAIL_LOCOTYPE_UNKNOWN           0x00
#define OORAIL_LOCOTYPE_STEAM             0x10
#define OORAIL_LOCOTYPE_DIESEL            0x20
#define OORAIL_LOCOTYPE_ELECTRIC          0x30
#define OORAIL_LOCOTYPE_GT                0x40
#define OORAIL_LOCOTYPE_DMU               0x50
#define OORAIL_LOCOTYPE_EMU               0x60
#define OORAIL_LOCOTYPE_DEMU              0x70

/* Profile defaults */
#define OORAIL_MAX_PROFILES 7
#define OORAIL_DEFAULT_MAX 1023
#define OORAIL_DEFAULT_MAXSPEED 984
#define OORAIL_DEFAULT_SHUNT 56
#define OORAIL_DEFAULT_SPEEDSTEP 8
#define OORAIL_DEFAULT_BRAKE 32

struct oorail_loco_profile_t {
  unsigned int mfreq;               /* PWM frequency to use */
  unsigned int pidle;               /* PWM duty cycle value for idle - maximum value before movement */
  unsigned int pstart;              /* PWM duty cycle minimum value to begin movement */
  unsigned int pmax;                /* Set the PWM upper limit - this cannot exceed 1023 */
  unsigned int pmaxspeed;           /* PWM duty cycle value to simulate the scale max speed of the locomotive */
  unsigned int pcrawl;              /* PWM duty cycle value to simulate very slow movement - coupling etc */
  unsigned int pshunt;              /* PWM duty cycle value to simulate a typical shunting speed */
  unsigned int pcoast;              /* PWM duty cycle value to simulate normal running speed */
  unsigned int asteps;              /* Acceleration - steps to jump with each acceleration increment */
  unsigned int dsteps;              /* Deceleration - steps to jump with each deceleration */
  unsigned int ainertia;            /* Acceleration Inertia - delay in ms before acceleration */
  unsigned int dinertia;            /* Deceleration Inertia - delay in ms before deceleration */
  unsigned int brakeforce;          /* Brake Force - amount to reduce duty cycle when applied */
  unsigned int type;                /* Loco type - Steam, Electric, Diesel, Gas Turbine, EMU, DMU, DEMU */
  unsigned int lococlass;           /* Loco class */
  unsigned int running_number;      /* Running number */
  unsigned int manufacturer;        /* Manufacturer */
};

struct oorail_state_t
{
  unsigned int brake;
  unsigned int stopped;
  unsigned int current_speed;
  unsigned int target_speed;
  unsigned int coast_speed;
  unsigned int max_speed;
  unsigned int oorail_mph;
  unsigned int travel;
};

struct oorail_pwm_t
{
  unsigned int ENA;       /* enable pin */
  unsigned int IN1;       /* input 1 or 3 */
  unsigned int IN2;       /* input 2 or 4 */
  unsigned int mfreq;     /* pwm frequency suggested 28kHz which is 28000 */
  unsigned int mres;      /* bit resolution */
  unsigned int mchan;     /* pwm channel */
};

struct oorail_track_module_t
{
  oorail_state_t track_state[OORAIL_TRACK_MODULE_OUTPUT];
  oorail_pwm_t track_pwm[OORAIL_TRACK_MODULE_OUTPUT];
  unsigned int track_enabled[OORAIL_TRACK_MODULE_OUTPUT];
};

struct oorail_profile_t 
{
  oorail_loco_profile_t loco[OORAIL_MAX_PROFILES];
};

#if defined(OORAIL_FEATURE_WIFI_LED)
const int WIFI_LED = 2;  /* Built-In WiFi LED pin */
#endif

const int ENA = 19;      /* ENA pin for control of track A */
const int ENB = 4;       /* Disabled */
const int IN1 = 18;      /* IN1 for track A */
const int IN2 = 5;       /* IN2 for track A */
const int IN3 = 17;      /* IN3 for track B */
const int IN4 = 16;      /* IN4 for track B */

/* WiFi credentials */
const char* ssid = "REPLACE-ME-WITH-YOUR-WIFI-SSID";
const char* password = "REPLACE-ME-WITH-YOUR-WIFI-PASSWORD";

/* track modules */
oorail_track_module_t oorail_tm;

/* profiles */
oorail_profile_t oorail_profile;


/* Create web server instance */
#if !defined(OORAIL_FEATURE_ASYNC_WEB)
WebServer server(OORAIL_PORT);
#endif

/* HTTP Server */
void oorail_web_init() {
  /* initialize API end points */
  /* Endpoints:
   *  
   *  Current API version is version 1
   *  
   *  /api/<version>/<module>/...
   *  
   *  Mappings:
   *  
   *  /debug  ---> /api/<version>/<module>/debug
   *  /ping   ---> /api/<version>/<module>/hc
   *  /status ---> /api/<version>/<module>/status
   *  /       ---> /api/<version>/<module>/info
   *  
   *  This module is Track Module (tm):
   *  
   *  /api/1/tm/
   *  
   *  Note: Initial release does not support ASYNC web requests.
   *        Only one request at a time will be processed.
   *        Subsequent requests can override in-progress requests.
   *        For example, if you sent the speed to 500 and then while its
   *        accelerating to 500, you set the speed to 250. It will either
   *        accelerate or decelerate to 250, depending on the current speed.
   */
#if !defined(OORAIL_FEATURE_ASYNC_WEB)
  server.on("/", HTTP_GET, oorail_web_info);
  server.on("/debug", HTTP_GET, oorail_web_debug);
  server.on("/ping", HTTP_GET, oorail_web_hc);
  server.on("/status", HTTP_GET, oorail_web_debug);
  server.on("/api/1/tm/debug", HTTP_GET, oorail_web_debug);
  server.on("/api/1/tm/hc", HTTP_GET, oorail_web_hc);
  server.on("/api/1/tm/info", HTTP_GET, oorail_web_info);
  server.on("/api/1/tm/status", HTTP_GET, oorail_web_debug);
  server.on("/api/1/tm/profile", HTTP_GET, oorail_web_profile);
  server.on("/api/1/tm/profile/hornby", HTTP_GET, oorail_web_profile_hornby);
  server.on("/api/1/tm/profile/heljan", HTTP_GET, oorail_web_profile_heljan);
  server.on("/api/1/tm/profile/dapol", HTTP_GET, oorail_web_profile_dapol);
  server.on("/api/1/tm/profile/bachmann", HTTP_GET, oorail_web_profile_bachmann);
  server.on("/api/1/tm/profile/custom", HTTP_GET, oorail_web_profile_custom);
  server.on("/api/1/tm/0/speed/current", HTTP_GET, oorail_web_askspeed_a);
  server.on("/api/1/tm/1/speed/current", HTTP_GET, oorail_web_askspeed_b);
  server.on("/api/1/tm/0/speed/up", HTTP_GET, oorail_web_accel_a);
  server.on("/api/1/tm/1/speed/up", HTTP_GET, oorail_web_accel_b);
  server.on("/api/1/tm/0/speed/down", HTTP_GET, oorail_web_decel_a);
  server.on("/api/1/tm/1/speed/down", HTTP_GET, oorail_web_decel_b);
  server.on("/api/1/tm/0/speed/stop", HTTP_GET, oorail_web_stop_a);
  server.on("/api/1/tm/1/speed/stop", HTTP_GET, oorail_web_stop_b);  
  server.on("/api/1/tm/0/stop", HTTP_GET, oorail_web_estop_a);    
  server.on("/api/1/tm/1/stop", HTTP_GET, oorail_web_estop_b);
  server.on("/api/1/tm/stop", HTTP_GET, oorail_web_estop_all);
  server.on("/api/1/tm/0/coast", HTTP_GET, oorail_web_coast_a);
  server.on("/api/1/tm/1/coast", HTTP_GET, oorail_web_coast_b);
  server.on("/api/1/tm/0/shunt", HTTP_GET, oorail_web_shunt_a);
  server.on("/api/1/tm/1/shunt", HTTP_GET, oorail_web_shunt_b);  
  server.on("/api/1/tm/0/crawl", HTTP_GET, oorail_web_crawl_a);
  server.on("/api/1/tm/1/crawl", HTTP_GET, oorail_web_crawl_b);  
  server.on("/api/1/tm/0/full", HTTP_GET, oorail_web_full_a);
  server.on("/api/1/tm/1/full", HTTP_GET, oorail_web_full_b);  
  server.on("/api/1/tm/0/brake", HTTP_GET, oorail_web_brake_a);
  server.on("/api/1/tm/1/brake", HTTP_GET, oorail_web_brake_b);  
  server.on("/api/1/tm/0/brake/on", HTTP_GET, oorail_web_brakeon_a);
  server.on("/api/1/tm/1/brake/on", HTTP_GET, oorail_web_brakeon_b);  
  server.on("/api/1/tm/0/brake/off", HTTP_GET, oorail_web_brakeoff_a);
  server.on("/api/1/tm/1/brake/off", HTTP_GET, oorail_web_brakeoff_b);  
  server.on("/api/1/tm/0/travel/up", HTTP_GET, oorail_web_travel_a);
  server.on("/api/1/tm/1/travel/up", HTTP_GET, oorail_web_travel_b);
  server.on("/api/1/tm/0/travel/down", HTTP_GET, oorail_web_travel_a);
  server.on("/api/1/tm/1/travel/down", HTTP_GET, oorail_web_travel_b);
  server.onNotFound(oorail_web_err_notfound);
  server.begin();
#endif
  Serial.println(" * HTTP server initialized ");
}

/* API routing functions for track A/B */
void oorail_web_askspeed_a() {
  oorail_web_askspeed(OORAIL_TRACK_A);
}

void oorail_web_askspeed_b() {
  oorail_web_askspeed(OORAIL_TRACK_B);
}

void oorail_web_accel_a() {
  oorail_web_accel(OORAIL_TRACK_A);
}

void oorail_web_accel_b() {
  oorail_web_accel(OORAIL_TRACK_B);
}

void oorail_web_decel_a() {
  oorail_web_decel(OORAIL_TRACK_A);
}

void oorail_web_decel_b() {
  oorail_web_decel(OORAIL_TRACK_B);
}

void oorail_web_stop_a() {
  oorail_web_stop(OORAIL_TRACK_A);
}

void oorail_web_stop_b() {
  oorail_web_stop(OORAIL_TRACK_B);
}

void oorail_web_coast_a() {
  oorail_web_coast(OORAIL_TRACK_A);
}

void oorail_web_coast_b() {
  oorail_web_coast(OORAIL_TRACK_B);
}

void oorail_web_shunt_a() {
  oorail_web_shunt(OORAIL_TRACK_A);
}

void oorail_web_shunt_b() {
  oorail_web_shunt(OORAIL_TRACK_B);
}

void oorail_web_crawl_a() {
  oorail_web_crawl(OORAIL_TRACK_A);
}

void oorail_web_crawl_b() {
  oorail_web_crawl(OORAIL_TRACK_B);
}

void oorail_web_full_a() {
  oorail_web_full(OORAIL_TRACK_A);
}

void oorail_web_full_b() {
  oorail_web_full(OORAIL_TRACK_B);
}


void oorail_web_estop_a() {
  oorail_web_estop(OORAIL_TRACK_A);
}

void oorail_web_estop_b() {
  oorail_web_estop(OORAIL_TRACK_B);
}

void oorail_web_estop_all() {
  oorail_web_estop(OORAIL_TRACK_A);
  oorail_web_estop(OORAIL_TRACK_B);
}

void oorail_web_brake_a() {
  oorail_web_brake(OORAIL_TRACK_A);
}

void oorail_web_brake_b() {
  oorail_web_brake(OORAIL_TRACK_B);
}

void oorail_web_brakeon_a() {
  oorail_web_brakeon(OORAIL_TRACK_A);
}

void oorail_web_brakeon_b() {
  oorail_web_brakeon(OORAIL_TRACK_B);
}

void oorail_web_brakeoff_a() {
  oorail_web_brakeoff(OORAIL_TRACK_A);
}

void oorail_web_brakeoff_b() {
  oorail_web_brakeoff(OORAIL_TRACK_B);
}

#if !defined(OORAIL_FEATURE_ASYNC_WEB)
void oorail_web_travel_a() {
  unsigned int travel = 0;
  char *upurl = "/api/1/tm/0/travel/up";
  char testurl[24];
  String requri = server.uri();

  requri.toCharArray(testurl, sizeof(testurl));

  if (!strncasecmp(upurl, testurl, strlen(upurl))) {
    travel = OORAIL_TRACK_FORWARD;
  } else {
    travel = OORAIL_TRACK_REVERSE;
  }
  
  oorail_web_travel(OORAIL_TRACK_A, travel);
}

void oorail_web_travel_b() {
  unsigned int travel = 0;
  char *upurl = "/api/1/tm/1/travel/up";
  char testurl[24];
  String requri = server.uri();

  requri.toCharArray(testurl, sizeof(testurl));

  if (!strncasecmp(upurl,testurl, strlen(upurl))) {
    travel = OORAIL_TRACK_REVERSE;
  } else {
    travel = OORAIL_TRACK_FORWARD;
  }

  oorail_web_travel(OORAIL_TRACK_B, travel);
}
#endif

/* URI Endpoint Handler Functions */
void oorail_web_askspeed(unsigned int track) {
  char response[255];
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  sprintf(response,"Current speed is %d. Target speed is %d\r\n", 
          oorail_tm.track_state[track].current_speed, oorail_tm.track_state[track].target_speed);
  server.send(200, "text/plain", response);
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_accel(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1); 
#endif
  if ((oorail_tm.track_state[track].current_speed + oorail_profile.loco[track].asteps) <= (oorail_tm.track_state[track].max_speed)) {
    oorail_tm.track_state[track].target_speed = oorail_tm.track_state[track].current_speed + oorail_profile.loco[track].asteps; 
    server.send(200, "text/plain", "oorail-ddc-basic accelerate command received");
  } else if ((oorail_tm.track_state[track].current_speed + 1) <= (oorail_tm.track_state[track].max_speed)) {
    oorail_tm.track_state[track].target_speed = oorail_tm.track_state[track].current_speed + 1; 
    server.send(200, "text/plain", "oorail-ddc-basic accelerate command received");   
  } else {
    server.send(403, "text/plain", "Forbidden - Maximum Speed Reached");       
  }
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_decel(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  server.send(200, "text/plain", "oorail-ddc-basic decelerate command received");
  if (oorail_tm.track_state[track].current_speed != 0) {
    if (oorail_tm.track_state[track].current_speed  >= oorail_profile.loco[track].dsteps) {
      oorail_tm.track_state[track].target_speed = oorail_tm.track_state[track].current_speed - oorail_profile.loco[track].dsteps;
    } else {
      oorail_tm.track_state[track].target_speed = oorail_tm.track_state[track].current_speed - 1;
    }
  } else {
    Serial.println("**DEBUG**: current speed is zero");
  }
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_stop(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  server.send(200, "text/plain", "oorail-ddc-basic stop command received");
  oorail_tm.track_state[track].target_speed = 0;
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_estop(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  server.send(200, "text/plain", "oorail-ddc-basic estop command received");
  oorail_tm.track_state[track].target_speed = 0;
  oorail_ctl_set_speed(track, 0);
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_coast(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  server.send(200, "text/plain", "oorail-ddc-basic coast command received");
  oorail_tm.track_state[track].target_speed =  oorail_tm.track_state[track].coast_speed;
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_shunt(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  server.send(200, "text/plain", "oorail-ddc-basic shunt command received");
  oorail_tm.track_state[track].target_speed = oorail_profile.loco[track].pshunt;
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);  
#endif
}

void oorail_web_crawl(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  server.send(200, "text/plain", "oorail-ddc-basic crawl command received");
  oorail_tm.track_state[track].target_speed = oorail_profile.loco[track].pcrawl;
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);  
#endif
}

void oorail_web_full(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  server.send(200, "text/plain", "oorail-ddc-basic full command received");
  oorail_tm.track_state[track].target_speed = oorail_profile.loco[track].pmaxspeed;
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);  
#endif
}

void oorail_web_brakeon(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  if (oorail_tm.track_state[track].brake == 0) {
    server.send(200, "text/plain", "oorail-ddc-basic apply brake command received");
    oorail_tm.track_state[track].brake = 1;
  } else {
    server.send(403, "text/plain", "Forbidden Brake Already On");    
  }
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_brakeoff(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  if (oorail_tm.track_state[track].brake == 1) {
    server.send(200, "text/plain", "oorail-ddc-basic release brake command received");
    oorail_tm.track_state[track].brake = 0;
  } else {
    server.send(403, "text/plain", "Forbidden Brake Already Off");    
  }
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_brake(unsigned int track) {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  if (oorail_tm.track_state[track].brake == 0) {
    server.send(200, "text/plain", "oorail-ddc-basic apply brake command received");
    if ((oorail_tm.track_state[track].current_speed - oorail_profile.loco[track].brakeforce) >= 0) {
      oorail_ctl_set_speed(track, oorail_tm.track_state[track].current_speed - oorail_profile.loco[track].brakeforce);
    } else {
      oorail_ctl_set_speed(track, 0);
    }
  } else {
    server.send(403, "text/plain", "Forbidden Brake Already On");    
  }
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_travel(unsigned int track, unsigned int travel) {
  char response[256];
  char whichway[8];
  unsigned int travel_changed = 0;
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  if (oorail_tm.track_state[track].travel != travel) {
    oorail_tm.track_state[track].travel = travel;
    travel_changed = 1;
  }
  if (((track == OORAIL_TRACK_A) && (oorail_tm.track_state[track].travel == OORAIL_TRACK_FORWARD)) ||
      ((track == OORAIL_TRACK_B) && (oorail_tm.track_state[track],travel == OORAIL_TRACK_REVERSE))) {
    sprintf(whichway,"Forward"); 
  } else {
    sprintf(whichway,"Reverse");
  }
  sprintf(response,"\r\ncmd: change direction - track %s - direction %s - changed - %s\r\n",
    (track == OORAIL_TRACK_A) ? "A" : "B", whichway, travel_changed ? "Yes" : "No");
  server.send(200, "text/plain", response);
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_debug() {
  char response[1024];
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  sprintf(response, "\r\nTrack A\r\nbrake = %d\r\nstopped = %d\r\ncurrent = %d\r\ntarget = %d\r\ncoast = %d\r\nmax = %d\r\n\r\n\r\nTrack B\r\nbrake = %d\r\nstopped = %d\r\ncurrent = %d\r\ntarget = %d\r\ncoast = %d\r\nmax = %d\r\n\r\n",
    oorail_tm.track_state[OORAIL_TRACK_A].brake,oorail_tm.track_state[OORAIL_TRACK_A].stopped, oorail_tm.track_state[OORAIL_TRACK_A].current_speed, 
    oorail_tm.track_state[OORAIL_TRACK_A].target_speed, oorail_tm.track_state[OORAIL_TRACK_A].coast_speed,
    oorail_tm.track_state[OORAIL_TRACK_A].max_speed,
    oorail_tm.track_state[OORAIL_TRACK_B].brake, oorail_tm.track_state[OORAIL_TRACK_B].stopped, oorail_tm.track_state[OORAIL_TRACK_B].current_speed, 
    oorail_tm.track_state[OORAIL_TRACK_B].target_speed, oorail_tm.track_state[OORAIL_TRACK_B].coast_speed,
    oorail_tm.track_state[OORAIL_TRACK_B].max_speed);
  server.send(200, "text/plain", response);
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);  
#endif
}

/*
 * TODO - Add WIFI_LED support to profile functions
 */

void oorail_web_profile() {
  server.send(200, "text/plain", "\r\nNOT IMPLEMENTED\r\n");
}

void oorail_web_profile_bachmann() {
  oorail_profile_load(2);
  server.send(200, "text/plain", "\r\nBachmann Profile Loaded\r\n");
}

void oorail_web_profile_custom() {
  oorail_profile_load(3);
  server.send(200, "text/plain", "\r\nCustom Profile Loaded\r\n");
}

void oorail_web_profile_dapol() {
  oorail_profile_load(4);
  server.send(200, "text/plain", "\r\nDepol Profile Loaded\r\n");  
}

void oorail_web_profile_heljan() {
  oorail_profile_load(5);
  server.send(200, "text/plain", "\r\nHeljan Profile Loaded\r\n");  
}

void oorail_web_profile_hornby() {
  oorail_profile_load(6);
  server.send(200, "text/plain", "\r\nHornby Profile Loaded\r\n");  
}

void oorail_web_info() {
  char response[1024];
  String LocalIP = String() + WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  sprintf(response,"\r\n%s, version %s\r\n%s\r\n\r\nLicensed under %s\r\n\r\nModule IP: %s\r\n\r\nTrack A: %s\r\nTrack B: %s\r\n\r\n",
    OORAIL_PROJECT, OORAIL_VERSION, OORAIL_COPYRIGHT, OORAIL_LICENSE, LocalIP, (oorail_tm.track_enabled[OORAIL_TRACK_A] == 1) ? "Enabled" : "Disabled",
    (oorail_tm.track_enabled[OORAIL_TRACK_B] == 1) ? "Enabled" : "Disabled");
    server.send(200, "text/plain", response);
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

void oorail_web_hc() {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  server.send(200, "text/plain", "\r\n...PONG...\r\n");
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}

#if !defined(OORAIL_FEATURE_ASYNC_WEB)
void oorail_web_err_notfound() {
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 1);
#endif
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
#if defined(OORAIL_FEATURE_WIFI_LED)
  digitalWrite(WIFI_LED, 0);
#endif
}
#endif
/* END HTTP SERVER */

void oorail_wifi()
{
  Serial.println(" * Connecting to WiFi... ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
}

/* Motor  Control */
void oorail_ctl_set_speed(unsigned int track, unsigned int speed) {
  Serial.print("Speed ");
  Serial.print(speed);
  Serial.println("");

  ledcWrite(oorail_tm.track_pwm[track].mchan, speed);
  oorail_tm.track_state[track].current_speed = speed;
}

void oorail_ctl_speed(unsigned int track) {
  unsigned int inertia = 0;
  
  if (oorail_tm.track_state[track].target_speed == oorail_tm.track_state[track].current_speed) return;

  if (oorail_tm.track_state[track].target_speed > oorail_tm.track_state[track].current_speed) {
    /* Accelerating */
    /* Check if we are incrementing single step or speed step */
    if ((oorail_tm.track_state[track].current_speed + oorail_profile.loco[track].asteps) 
          <= oorail_tm.track_state[track].target_speed) {
      oorail_ctl_set_speed(track, oorail_tm.track_state[track].current_speed + oorail_profile.loco[track].asteps);
    } else {
      oorail_ctl_set_speed(track, oorail_tm.track_state[track].current_speed + 1);
    }
    inertia = oorail_profile.loco[track].ainertia;
  } else if (oorail_tm.track_state[track].target_speed < oorail_tm.track_state[track].current_speed) {
    /* Decelerating */
    /* Check if we are using single or speed step */
    if ((oorail_tm.track_state[track].current_speed - oorail_profile.loco[track].dsteps)
          >= oorail_tm.track_state[track].target_speed) {
      oorail_ctl_set_speed(track, (oorail_tm.track_state[track].current_speed - oorail_profile.loco[track].dsteps));
    } else {
      oorail_ctl_set_speed(track, (oorail_tm.track_state[track].current_speed - 1));
    }
    inertia = oorail_profile.loco[track].dinertia;
  } else {
    /* Problem */
    Serial.println("WATCHDOG in oorail_ctl_speed");
    Serial.print("Target and Current Speed mismatch on ");
    if (track == OORAIL_TRACK_A) {
      Serial.print("track A ");
    } else if (track ==  OORAIL_TRACK_B) {
      Serial.print("track B ");
    } else {
      Serial.print("track unknown ");
    }
    Serial.print(oorail_tm.track_state[track].target_speed);
    Serial.print(" ");
    Serial.print(oorail_tm.track_state[track].current_speed);
    Serial.println(" ");
    return;
  }
  Serial.println("");
  Serial.print("Target Speed = ");
  Serial.print(oorail_tm.track_state[track].target_speed);
  Serial.print(" | Current Speed = ");
  Serial.println(oorail_tm.track_state[track].current_speed);
  delay(inertia);  /* TODO - replace with accel / decel inertia value */
  
  if ((oorail_tm.track_state[track].current_speed == 0) 
      && (oorail_tm.track_state[track].stopped = 0)) {
    oorail_tm.track_state[track].stopped = 1;
  } else if ((oorail_tm.track_state[track].current_speed > 0)  
      && (oorail_tm.track_state[track].stopped == 1)) {
    oorail_tm.track_state[track].stopped = 0;
  }
}

void oorail_track_module_init() {
  int i = 0;
  Serial.println(" * Initializing Profile ");

  memset(&oorail_tm, 0, sizeof(oorail_track_module_t));
  
  /* Initialize track */
  oorail_tm.track_enabled[OORAIL_TRACK_A] = OORAIL_TRUE;
  oorail_tm.track_enabled[OORAIL_TRACK_B] = OORAIL_FALSE;

  /* Initialize pwm pins */
  oorail_tm.track_pwm[OORAIL_TRACK_A].ENA = ENA;
  oorail_tm.track_pwm[OORAIL_TRACK_B].ENA = ENB;
  oorail_tm.track_pwm[OORAIL_TRACK_A].IN1 = IN1;
  oorail_tm.track_pwm[OORAIL_TRACK_B].IN1 = IN3;
  oorail_tm.track_pwm[OORAIL_TRACK_A].IN2 = IN2;
  oorail_tm.track_pwm[OORAIL_TRACK_B].IN2 = IN4;

  /* initialize pwm config */
  oorail_tm.track_pwm[OORAIL_TRACK_A].mfreq = OORAIL_PWM_FREQ_DEFAULT;
  oorail_tm.track_pwm[OORAIL_TRACK_A].mres = OORAIL_PWM_RES_DEFAULT;
  oorail_tm.track_pwm[OORAIL_TRACK_A].mchan = OORAIL_PWM_CHAN_A;
  oorail_tm.track_pwm[OORAIL_TRACK_B].mfreq = OORAIL_PWM_FREQ_DEFAULT;
  oorail_tm.track_pwm[OORAIL_TRACK_B].mres = OORAIL_PWM_RES_DEFAULT;
  oorail_tm.track_pwm[OORAIL_TRACK_B].mchan = OORAIL_PWM_CHAN_B;

  /* initialize state */
  for (i = 0; i < OORAIL_TRACK_MODULE_OUTPUT; i++) {
    oorail_tm.track_state[i].brake = OORAIL_BRAKE_ON;
    oorail_tm.track_state[i].stopped = OORAIL_TRUE;
    oorail_tm.track_state[i].current_speed = 0;
    oorail_tm.track_state[i].coast_speed  = (OORAIL_DEFAULT_MAXSPEED / 2);
    oorail_tm.track_state[i].target_speed = 0;
    oorail_tm.track_state[i].max_speed = OORAIL_DEFAULT_MAXSPEED;
    oorail_tm.track_state[i].oorail_mph = oorail_tm.track_state[i].current_speed;
    if (i == OORAIL_TRACK_A) {
      oorail_tm.track_state[i].travel = OORAIL_TRACK_FORWARD;
    } else {
      oorail_tm.track_state[i].travel = OORAIL_TRACK_REVERSE;
    }
  }
}

/*
 * TODO - add these functions to be called if mfreq changes on profile load
 */

void oorail_update_track_pwm()
{
  oorail_tm.track_pwm[OORAIL_TRACK_A].mfreq = oorail_profile.loco[OORAIL_TRACK_A].mfreq;
  oorail_tm.track_pwm[OORAIL_TRACK_B].mfreq = oorail_profile.loco[OORAIL_TRACK_B].mfreq;
}

void oorail_update_l283d(void)
{
  Serial.println(" * Re-Initializing L283d module...");
  
  Serial.println(" * Re-Initializing Track A...");
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_A].ENA, OUTPUT);
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_A].IN1, OUTPUT);
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_A].IN2, OUTPUT);

  ledcSetup(oorail_tm.track_pwm[OORAIL_TRACK_A].mchan, 
            oorail_tm.track_pwm[OORAIL_TRACK_A].mfreq, 
            oorail_tm.track_pwm[OORAIL_TRACK_A].mres);
  ledcAttachPin(oorail_tm.track_pwm[OORAIL_TRACK_A].ENA, oorail_tm.track_pwm[OORAIL_TRACK_A].mchan);
  ledcWrite(oorail_tm.track_pwm[OORAIL_TRACK_A].mchan, oorail_tm.track_state[OORAIL_TRACK_A].current_speed);


  Serial.println(" * Re-Initializing Track B...");
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_B].ENA, OUTPUT);
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_B].IN1, OUTPUT);
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_B].IN2, OUTPUT);

  ledcSetup(oorail_tm.track_pwm[OORAIL_TRACK_B].mchan, 
            oorail_tm.track_pwm[OORAIL_TRACK_B].mfreq, 
            oorail_tm.track_pwm[OORAIL_TRACK_B].mres);
  ledcAttachPin(oorail_tm.track_pwm[OORAIL_TRACK_B].ENA, oorail_tm.track_pwm[OORAIL_TRACK_B].mchan);
  ledcWrite(oorail_tm.track_pwm[OORAIL_TRACK_B].mchan, oorail_tm.track_state[OORAIL_TRACK_B].current_speed);

  return;
}

void oorail_setup_l283d(void)
{
  Serial.println(" * Initializing L283d module...");
  
  Serial.println(" * Initializing Track A...");
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_A].ENA, OUTPUT);
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_A].IN1, OUTPUT);
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_A].IN2, OUTPUT);

  ledcSetup(oorail_tm.track_pwm[OORAIL_TRACK_A].mchan, 
            oorail_tm.track_pwm[OORAIL_TRACK_A].mfreq, 
            oorail_tm.track_pwm[OORAIL_TRACK_A].mres);
  ledcAttachPin(oorail_tm.track_pwm[OORAIL_TRACK_A].ENA, oorail_tm.track_pwm[OORAIL_TRACK_A].mchan);
  ledcWrite(oorail_tm.track_pwm[OORAIL_TRACK_A].mchan, 0); // start stopped


  Serial.println(" * Initializing Track B...");
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_B].ENA, OUTPUT);
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_B].IN1, OUTPUT);
  pinMode(oorail_tm.track_pwm[OORAIL_TRACK_B].IN2, OUTPUT);

  ledcSetup(oorail_tm.track_pwm[OORAIL_TRACK_B].mchan, 
            oorail_tm.track_pwm[OORAIL_TRACK_B].mfreq, 
            oorail_tm.track_pwm[OORAIL_TRACK_B].mres);
  ledcAttachPin(oorail_tm.track_pwm[OORAIL_TRACK_B].ENA, oorail_tm.track_pwm[OORAIL_TRACK_B].mchan);
  ledcWrite(oorail_tm.track_pwm[OORAIL_TRACK_B].mchan, 0); // start stopped

  return;
}

void oorail_profile_init(void)
{
  unsigned int i = 0;
  
  oorail_profile.loco[0].manufacturer = OORAIL_MANUFACTURER_HORNBY;       /* Track A profile default */
  oorail_profile.loco[1].manufacturer = OORAIL_MANUFACTURER_TRACKSIDE3D;  /* Track B profile default */
  oorail_profile.loco[2].manufacturer = OORAIL_MANUFACTURER_BACHMANN;     /* Bachmann Stored Profile */
  oorail_profile.loco[3].manufacturer = OORAIL_MANUFACTURER_TRACKSIDE3D;  /* Custom Stored Profile */
  oorail_profile.loco[4].manufacturer = OORAIL_MANUFACTURER_DAPOL;        /* Dapol Stored Profile */
  oorail_profile.loco[5].manufacturer = OORAIL_MANUFACTURER_HELJAN;       /* Heljan Stored Profile */
  oorail_profile.loco[6].manufacturer = OORAIL_MANUFACTURER_HORNBY;       /* Hornby Stored Profile */

  /* TODO: Need to tune these values */

  for (i = 0; i < OORAIL_MAX_PROFILES; i++) {
    switch (oorail_profile.loco[i].manufacturer) {
      case (OORAIL_MANUFACTURER_HORNBY):
        oorail_profile.loco[i].mfreq = OORAIL_PWM_FREQ_DEFAULT;
        oorail_profile.loco[i].pidle = 32;
        oorail_profile.loco[i].pstart = 48;
        oorail_profile.loco[i].pmax = 768;
        oorail_profile.loco[i].pmaxspeed = 512;
        oorail_profile.loco[i].pcrawl = 64;
        oorail_profile.loco[i].pshunt = 96;
        oorail_profile.loco[i].pcoast = 468;
        oorail_profile.loco[i].asteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].dsteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].ainertia = 640;
        oorail_profile.loco[i].dinertia = 768;
        oorail_profile.loco[i].brakeforce = OORAIL_DEFAULT_BRAKE;
        oorail_profile.loco[i].type = OORAIL_LOCOTYPE_UNKNOWN;
        oorail_profile.loco[i].lococlass = OORAIL_FALSE;
        oorail_profile.loco[i].running_number = OORAIL_FALSE;
        break;

      case (OORAIL_MANUFACTURER_HELJAN):
        oorail_profile.loco[i].mfreq = OORAIL_PWM_FREQ_DEFAULT;
        oorail_profile.loco[i].pidle = 400;
        oorail_profile.loco[i].pstart = 480;
        oorail_profile.loco[i].pmax = 1023;
        oorail_profile.loco[i].pmaxspeed = 1023;
        oorail_profile.loco[i].pcrawl = 520;
        oorail_profile.loco[i].pshunt = 700;
        oorail_profile.loco[i].pcoast = 850;
        oorail_profile.loco[i].asteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].dsteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].ainertia = 450;
        oorail_profile.loco[i].dinertia = 450;
        oorail_profile.loco[i].brakeforce = OORAIL_DEFAULT_BRAKE;
        oorail_profile.loco[i].type = OORAIL_LOCOTYPE_UNKNOWN;
        oorail_profile.loco[i].lococlass = OORAIL_FALSE;
        oorail_profile.loco[i].running_number = OORAIL_FALSE;
        break;

      case (OORAIL_MANUFACTURER_DAPOL):
        oorail_profile.loco[i].mfreq = OORAIL_PWM_FREQ_DEFAULT;
        oorail_profile.loco[i].pidle = 320;
        oorail_profile.loco[i].pstart = 360;
        oorail_profile.loco[i].pmax = 1023;
        oorail_profile.loco[i].pmaxspeed = 1023;
        oorail_profile.loco[i].pcrawl = 380;
        oorail_profile.loco[i].pshunt = 496;
        oorail_profile.loco[i].pcoast = 815;
        oorail_profile.loco[i].asteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].dsteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].ainertia = 450;
        oorail_profile.loco[i].dinertia = 450;
        oorail_profile.loco[i].brakeforce = OORAIL_DEFAULT_BRAKE;
        oorail_profile.loco[i].type = OORAIL_LOCOTYPE_UNKNOWN;
        oorail_profile.loco[i].lococlass = OORAIL_FALSE;
        oorail_profile.loco[i].running_number = OORAIL_FALSE;
        break;

      case (OORAIL_MANUFACTURER_BACHMANN):
        oorail_profile.loco[i].mfreq = OORAIL_PWM_FREQ_DEFAULT;
        oorail_profile.loco[i].pidle = 320;
        oorail_profile.loco[i].pstart = 360;
        oorail_profile.loco[i].pmax = 1023;
        oorail_profile.loco[i].pmaxspeed = 1023;
        oorail_profile.loco[i].pcrawl = 380;
        oorail_profile.loco[i].pshunt = 420;
        oorail_profile.loco[i].pcoast = 512;
        oorail_profile.loco[i].asteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].dsteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].ainertia = 512;
        oorail_profile.loco[i].dinertia = 512;
        oorail_profile.loco[i].brakeforce = OORAIL_DEFAULT_BRAKE;
        oorail_profile.loco[i].type = OORAIL_LOCOTYPE_UNKNOWN;
        oorail_profile.loco[i].lococlass = OORAIL_FALSE;
        oorail_profile.loco[i].running_number = OORAIL_FALSE;
        break;

      default:
      case (OORAIL_MANUFACTURER_TRACKSIDE3D):
        oorail_profile.loco[i].mfreq = OORAIL_PWM_FREQ_DEFAULT;
        oorail_profile.loco[i].pidle = 48;
        oorail_profile.loco[i].pstart = 64;
        oorail_profile.loco[i].pmax = OORAIL_DEFAULT_MAX;
        oorail_profile.loco[i].pmaxspeed = OORAIL_DEFAULT_MAXSPEED;
        oorail_profile.loco[i].pcrawl = 96;
        oorail_profile.loco[i].pshunt = OORAIL_DEFAULT_SHUNT;
        oorail_profile.loco[i].pcoast = (OORAIL_DEFAULT_MAXSPEED/2);
        oorail_profile.loco[i].asteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].dsteps = OORAIL_DEFAULT_SPEEDSTEP;
        oorail_profile.loco[i].ainertia = 450;
        oorail_profile.loco[i].dinertia = 450;
        oorail_profile.loco[i].brakeforce = OORAIL_DEFAULT_BRAKE;
        oorail_profile.loco[i].type = OORAIL_LOCOTYPE_UNKNOWN;
        oorail_profile.loco[i].lococlass = OORAIL_FALSE;
        oorail_profile.loco[i].running_number = OORAIL_FALSE;
        break;
    }
  }  
  oorail_tm.track_state[OORAIL_TRACK_A].max_speed = oorail_profile.loco[OORAIL_TRACK_A].pmaxspeed;
  oorail_tm.track_state[OORAIL_TRACK_A].coast_speed = oorail_profile.loco[OORAIL_TRACK_A].pcoast;
}

/* NOTE: This only supports Track A initially */
void oorail_profile_load(unsigned int p)
{
  unsigned int i = 0;

  oorail_profile.loco[i].mfreq = oorail_profile.loco[p].mfreq;
  oorail_profile.loco[i].pidle = oorail_profile.loco[p].pidle;
  oorail_profile.loco[i].pstart = oorail_profile.loco[p].pstart;
  oorail_profile.loco[i].pmax = oorail_profile.loco[p].pmax;
  oorail_profile.loco[i].pmaxspeed = oorail_profile.loco[p].pmaxspeed;
  oorail_profile.loco[i].pcrawl = oorail_profile.loco[p].pcrawl;
  oorail_profile.loco[i].pshunt = oorail_profile.loco[p].pshunt;
  oorail_profile.loco[i].pcoast = oorail_profile.loco[p].pcoast;
  oorail_profile.loco[i].asteps = oorail_profile.loco[p].asteps;
  oorail_profile.loco[i].dsteps = oorail_profile.loco[p].dsteps;
  oorail_profile.loco[i].ainertia = oorail_profile.loco[p].ainertia;
  oorail_profile.loco[i].dinertia = oorail_profile.loco[p].dinertia;
  oorail_profile.loco[i].brakeforce = oorail_profile.loco[p].brakeforce;
  oorail_profile.loco[i].type = oorail_profile.loco[p].type;
  oorail_profile.loco[i].lococlass = oorail_profile.loco[p].lococlass;
  oorail_profile.loco[i].running_number = oorail_profile.loco[p].running_number;
  oorail_profile.loco[i].manufacturer = oorail_profile.loco[p].manufacturer;

  oorail_tm.track_state[OORAIL_TRACK_A].max_speed = oorail_profile.loco[OORAIL_TRACK_A].pmaxspeed;
  oorail_tm.track_state[OORAIL_TRACK_A].coast_speed = oorail_profile.loco[OORAIL_TRACK_A].pcoast;
}

void setup() {
#if defined(OORAIL_FEATURE_WIFI_LED)
  pinMode(WIFI_LED, OUTPUT);
#endif
  Serial.begin (115200);
  oorail_banner();
  memset(&oorail_tm, 0, sizeof(oorail_track_module_t));
  memset(&oorail_profile, 0, sizeof(oorail_profile_t));
  oorail_track_module_init();
  oorail_profile_init(); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  oorail_wifi();
  oorail_web_init();
  oorail_setup_l283d();
}

void loop() {
  if (oorail_tm.track_enabled[OORAIL_TRACK_A]) {
    /* Configure L283N to go in the direction of travel */
    if (oorail_tm.track_state[OORAIL_TRACK_A].travel == OORAIL_TRACK_FORWARD) {
      digitalWrite(oorail_tm.track_pwm[OORAIL_TRACK_A].IN1, OORAIL_TRACK_FORWARD_IN1);
      digitalWrite(oorail_tm.track_pwm[OORAIL_TRACK_A].IN2, OORAIL_TRACK_FORWARD_IN2);
    } else if (oorail_tm.track_state[OORAIL_TRACK_A].travel == OORAIL_TRACK_REVERSE) {
      digitalWrite(oorail_tm.track_pwm[OORAIL_TRACK_A].IN1, OORAIL_TRACK_REVERSE_IN1);
      digitalWrite(oorail_tm.track_pwm[OORAIL_TRACK_A].IN2, OORAIL_TRACK_REVERSE_IN2);
    }
    oorail_ctl_speed(OORAIL_TRACK_A);
  }
  if (oorail_tm.track_enabled[OORAIL_TRACK_B]) {
    /* Configure L283N to go in the direction of travel */
    if (oorail_tm.track_state[OORAIL_TRACK_B].travel == OORAIL_TRACK_FORWARD) {
      digitalWrite(oorail_tm.track_pwm[OORAIL_TRACK_B].IN1, OORAIL_TRACK_FORWARD_IN1);
      digitalWrite(oorail_tm.track_pwm[OORAIL_TRACK_B].IN2, OORAIL_TRACK_FORWARD_IN2);
    } else if (oorail_tm.track_state[OORAIL_TRACK_B].travel == OORAIL_TRACK_REVERSE) {
      digitalWrite(oorail_tm.track_pwm[OORAIL_TRACK_B].IN1, OORAIL_TRACK_REVERSE_IN1);
      digitalWrite(oorail_tm.track_pwm[OORAIL_TRACK_B].IN2, OORAIL_TRACK_REVERSE_IN2);
    }
    oorail_ctl_speed(OORAIL_TRACK_B);
  }
  server.handleClient();
}

unsigned int oorail_track_status(unsigned int track) {
  if (track >= OORAIL_TRACK_MODULE_OUTPUT) {
    return(OORAIL_TRACK_ERROR);
  }
  if (oorail_tm.track_enabled[track]) {
    return(OORAIL_TRUE);    
  } else {
    return(OORAIL_FALSE);
  }
}

void oorail_banner() {
  Serial.println("");
  Serial.println("");
  Serial.print(OORAIL_PROJECT);
  Serial.print(", version ");
  Serial.println(OORAIL_VERSION);
  Serial.println(OORAIL_COPYRIGHT);
  Serial.println("");
  Serial.println("For additional information visit:");
  Serial.println(" https://oorail.co.uk/tech/ ");
  Serial.println("");
  Serial.print("License: ");
  Serial.println(OORAIL_LICENSE);
  Serial.println("");
}
