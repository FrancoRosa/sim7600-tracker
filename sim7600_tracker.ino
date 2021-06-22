// - - - - - - - - - - - - - - - - - - - - - - - 
// Commands to compile/upload and monitor output
// arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// arduino-cli upload --port /dev/ttyACM0 --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// picocom /dev/ttyACM0 
// - - - - - - - - - - - - - - - - - - - - - - - 

#include <FreeRTOS_SAMD21.h>
#include <avr/dtostrf.h>
#include <FlashStorage.h>

#define led_pin 13
#define key_pin 8

volatile bool flagOK = false;
volatile bool flagERROR = false;
volatile bool flagREG = false;
volatile bool flagGNS = false;
volatile bool flagHTTPACT = false;
volatile bool flagDOWNLOAD = false;
volatile bool flagProcessing = false;

const int modem_size = 180;       // Size of Modem serila buffer
char modem_buffer[modem_size];    // Modem serial buffer 
char url_buffer[modem_size];    // Modem serial buffer 
char command_buffer[modem_size];    // Modem serial buffer 
char modem_char;                  // Modem serial char
volatile int modem_i;            // Modem serial index
volatile int edges[2];

char memory[20][modem_size];
// - - - - - - - - - - - - - - - - - - - - - - - 
const char mOK[]     = "OK";
const char mERROR[]  = "ERRO";
const char mCLOSED[] = "CLOS";
const char mCBC[] = "+CBC";
const char mCOP[] = "+COP";
const char mCSQ[] = "+CSQ";
const char mCGR[] = "+CGR";
const char mIPC[] = "+IPC";
const char mCGN[] = "+CGN";
const char mGSN[] = "8639";   // this is SIMCOMS id found on serial number
const char mRIN[] = "RING"; // when someones makes a call.. it should hang up
const char mHUP[] = "ATH";  // when someones makes a call.. it should hang up
// - - - - - - - - - - - - - - - - - - - - - - - 

TaskHandle_t Handle_rxTask;
TaskHandle_t Handle_txTask;
char latitude[13];
char longitude[13];
char date[12];
char time[9];
char lat_indicator[2];
char lng_indicator[2];
char altitude[7]; 
char speed[7]; 
char hdop[7];
char GSN[17];

volatile float lat;
volatile float lng;

typedef struct {
  char server[100];
  int stationary_period;
  int logging_period;
  int upload_period;
  bool recovery;
  bool valid;
} Config;

FlashStorage(storage, Config);

Config settings;

void checkConfig(){
  settings = storage.read();
  if (settings.valid) {
    Serial.print("... settings found");
  } 
  else {
    Serial.print("... settings not found");
    const char default_domain[] = "http://iotnetwork.com.au:5055/";
    memcpy(settings.server, default_domain, strlen(default_domain));
    settings.stationary_period = 600;
    settings.logging_period = 20;
    settings.upload_period = 240;
    settings.recovery = false;
    settings.valid = true;
    storage.write(settings);
    Serial.print("... settings saved");

  }
}

// - - - - - - - - Delay Helpers - - - - - - - - 
void osDelayUs(int us) {
  vTaskDelay( us / portTICK_PERIOD_US );  
}

void osDelayMs(int ms) {
  vTaskDelay( (ms * 1000) / portTICK_PERIOD_US );  
}

void osDelayS(int s) {
  while (s>0){
    osDelayMs(1000);
    s--;    
  }  
}
// - - - - - - - - - - - - - - - - - - - - - - - 

int find_chr(const char *text, const int start, const char chr) {
  char * pch;
	pch = (char*) memchr (&text[start], chr, strlen(text));
	if (pch != NULL) {
    return min(pch - text, strlen(text)-1);
  }
}

void find_edges(const char *text, int order, const char chr) {
	int start = 0;
	int end = 0;
	int i = 0;
	while ((find_chr(text, start, chr) != -1) && (i<=order)){
		start = end;
		end = find_chr(text, start+1, chr);
		i++;
	}
	edges[0] = start;
	edges[1] = end;
}

void split_chr(char *destination, const char *source, const char chr, const int part) {
	find_edges(source, part, chr);
	int len = edges[1] - edges[0] - 1;
  int i = 0;
  while(i < len){
    destination[i] = source[i + edges[0] + 1];
    i++;
  }
  destination[i]=0;
}


void modemPower() {
  Serial.println("... restart modem");
  digitalWrite(key_pin, LOW); osDelayS(3);
  digitalWrite(key_pin, HIGH); osDelayS(10);
}

// Read modem IMEI
void procGSN() {
  memcpy(GSN, modem_buffer, 15);
  Serial.print("... GSN: "); Serial.println(GSN);
}

float to_geo(char *coordinate, char *indicator) {
  char d_str[5]; 
  char m_str[12];
  int len = strlen(coordinate);
  if (len == 11) {
    memcpy(d_str, coordinate, 2);
    memmove(coordinate, coordinate+2, 9);
  } 
  if (len == 12) {
    memcpy(d_str, coordinate, 3);
    memmove(coordinate, coordinate+3, 9);
  }
  memcpy(m_str, coordinate, 9);
  float deg = atof(d_str);
  float min = atof(m_str);
  deg = deg + min / 60.0;

  if (indicator[0] == 'S' || indicator[0] == 'W') deg = 0 - deg;
  return deg;
}

void to_date(char *date) {
  // input: 190621
  // output: 2021-06-21
  char temp[12];
  temp[0] = '2';
  temp[1] = '0';
  temp[2] = date[4];
  temp[3] = date[5];
  temp[4] = '-';
  temp[5] = date[2];
  temp[6] = date[3];
  temp[7] = '-';
  temp[8] = date[0];
  temp[9] = date[1];
  temp[10] = 0;
  memcpy(date, temp, 11);
}

void to_time(char *time) {
  // input: 035032.0
  // output: 03:50:32
  char temp[10];
  temp[0] = time[0];
  temp[1] = time[1];
  temp[2] = ':';
  temp[3] = time[2];
  temp[4] = time[3];
  temp[5] = ':';
  temp[6] = time[4];
  temp[7] = time[5];
  temp[10] = 0;
  memcpy(time, temp, 10);

}

void procCGN() {
  flagGNS = false;
  if (modem_buffer[10]==':') {
    if (modem_buffer[12]=='2' || modem_buffer[12]=='3') { // if GNS is fixed
      flagGNS = true;
      split_chr(latitude, modem_buffer, ',', 4);      Serial.print("... latitude: ");      Serial.println(latitude);
      split_chr(longitude, modem_buffer, ',', 6);     Serial.print("... longitude: ");     Serial.println(longitude);
      split_chr(lat_indicator, modem_buffer, ',', 5); Serial.print("... lat_indicator: "); Serial.println(lat_indicator);
      split_chr(lng_indicator, modem_buffer, ',', 7); Serial.print("... lng_indicador: "); Serial.println(lng_indicator);
      lat = to_geo(latitude, lat_indicator);          Serial.print("... lat: ");           Serial.println(lat,6);
      lng = to_geo(longitude, lng_indicator);         Serial.print("... lng: ");           Serial.println(lng,6);
      dtostrf(lat, 7, 6, latitude);
      dtostrf(lng, 7, 6, longitude);
      split_chr(date, modem_buffer, ',', 8);          Serial.print("... date: ");          Serial.println(date);
      split_chr(time, modem_buffer, ',', 9);          Serial.print("... time: ");          Serial.println(time);
      to_date(date);                                  Serial.print("... date: ");          Serial.println(date);
      to_time(time);                                  Serial.print("... time: ");          Serial.println(time);
      split_chr(altitude, modem_buffer, ',', 10);     Serial.print("... altitude: ");      Serial.println(altitude);
      split_chr(speed, modem_buffer, ',', 11);        Serial.print("... speed: ");         Serial.println(speed);
      split_chr(hdop, modem_buffer, ',', 14);         Serial.print("... hdop: ");          Serial.println(hdop);
      
    } 
  }
} 

bool sendCommand(const char *command,int timeout) {
  Serial1.print("AT+");
  Serial1.print(command);
  Serial1.print("\r");
  return waitOk(timeout);
}

bool waitOk(int timeout) {
  flagOK = false;
  flagERROR= false;
  timeout= timeout*10;
  int t = 0;
  while (timeout > t)   {
    t++;
    if (flagOK || flagERROR) {
      osDelayMs(100);
      return true;
    }
    osDelayMs(100);
  }
  modemPower();
  return false;
}

void create_url() {
  // http://iotnetwork.com.au:5055/?id=863922031635619&lat=-13.20416&lon=-72.20898&timestamp=1624031099&hdop=12&altitude=3400&speed=10
  sprintf(
    url_buffer,
    "%s?id=%s&lat=%s&lon=%s&timestamp=%s%%20%s&hdop=%s&altitude=%s&speed=%s",
    settings.server, GSN, latitude, longitude, date, time, hdop, altitude, speed
  );
}

void create_command() {
  // http://iotnetwork.com.au:5055/?id=863922031635619&lat=-13.20416&lon=-72.20898&timestamp=1624031099&hdop=12&altitude=3400&speed=10
  sprintf(
    command_buffer,
    "HTTPPARA=\"URL\",\"%s\"",
    url_buffer
  );
}

void initHTTP() {
  sendCommand("HTTPINIT", 3);
}

void postHTTP() {
  create_command();
  sendCommand(command_buffer, 5);
  sendCommand("HTTPACTION=1", 15);
}

void stopHTTP() {
  sendCommand("HTTPTERM", 3);
}

bool smsConfig() {
  bool result = sendCommand("CSMS=0", 5) && 
  sendCommand("CPMS=\"ME\",\"ME\",\"ME\"", 5) &&
  sendCommand("CMGF=1", 5) &&
  sendCommand("CNMI=2,2", 5);
  return result;
}

static void task_rx_modem(void *pvParameters) {
  while (true) {
    while(Serial1.available() && !flagProcessing) {
      modem_char = Serial1.read();
      Serial.print(modem_char);
      modem_buffer[modem_i] = modem_char;
      modem_i++;
      if (modem_i >= modem_size) modem_i = 0;
      // if (modem_char=='>') flagPromt = true;
      if ((modem_i >= 2) && ((modem_char == '\n') || (modem_char == '\n')))
        {
          flagProcessing = true;
          modem_buffer[modem_i]='\0';
          if (memcmp(mOK,    modem_buffer,2)==0) flagOK=true;
          // if (memcmp(mERROR, modem_buffer,4)==0) flagERROR=true;
          // if (memcmp(mCLOSED,modem_buffer,4)==0) flagConn=false;
          // if (memcmp(mSend,  modem_buffer,6)==0) flagSend=true;
          // if (memcmp(mConn,  modem_buffer,9)==0) flagConn=true;
          // if (memcmp(mRIN,   modem_buffer,4)==0) flagRIN=true;
          if (memcmp(mCGN,   modem_buffer,4)==0) procCGN();
          if (memcmp(mGSN,   modem_buffer,4)==0) procGSN();
          modem_i=0;
          for(int i = 0; i < modem_size; i++) modem_buffer[i]=0;
          flagProcessing = false;
        }
    }
   osDelayUs(1);
  }
}

static void task_tx_modem(void *pvParameters) {
  while (true) {
    if (smsConfig()) {
      sendCommand("GSN", 3);
      sendCommand("CMEE=2", 10);
      sendCommand("CGNSSINFO", 10);
      digitalWrite(led_pin, !digitalRead(led_pin));
    }
    if (flagGNS) {
      create_url();
      initHTTP();
      postHTTP();
      osDelayS(10);
      stopHTTP();
    }
    osDelayS(50);
  }
}

void setup() {
  delay(1000); // keep this to avoid USB crash
  pinMode(led_pin, OUTPUT);
  pinMode(key_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);
  digitalWrite(key_pin, HIGH);
  Serial.begin(230400);
  delay(1000); // keep this to avoid USB crash
  Serial1.begin(115200);
  delay(1000); // keep this to avoid USB crash
  checkConfig();
  xTaskCreate(task_tx_modem, "txModem", 512, NULL, tskIDLE_PRIORITY + 2, &Handle_txTask);
  xTaskCreate(task_rx_modem, "rxModem", 512, NULL, tskIDLE_PRIORITY + 1, &Handle_rxTask);
  vTaskStartScheduler();

  while(true) {
    Serial.println("OS Failed! \n");
    delay(1000);
  }
}

void loop() {
  delay(1000);
}

// find for domain on memory, if not give default.
// find for interval on memory, if not, give default.

// Add interval counter and flag

// Stationary Heartbeat - Minutes
// Motion Logging rate - seconds
// Motion upload rate - minutes
// recovery mode, upload every 10s

// set recovery mode flag

// get RTC from IC

// save positions and send at once

// Variables to save in memory 
// #*,SERVER,http://iotnetwork.com.au:5055/
// #*,RECOVERY,true
// #*,RECOVERY,false
// #*,STATIONARY,1
// #*,LOGGING,30
// #*,UPLOAD,5