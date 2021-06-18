// - - - - - - - - - - - - - - - - - - - - - - - 
// Commands to compile/upload and monitor output
// arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// arduino-cli upload --port /dev/ttyACM0 --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// picocom /dev/ttyACM0 
// - - - - - - - - - - - - - - - - - - - - - - - 

#include <FreeRTOS_SAMD21.h>
#include <avr/dtostrf.h>

#define led_pin 13
#define key_pin 8
// #define Modem Serial1
// #define SerialUSB Serial

const char domain[] = "http://iotnetwork.com.au:5055/";

volatile bool flagOK = false;
volatile bool flagERROR = false;
volatile bool flagREG = false;
volatile bool flagGNS = false;
volatile bool flagHTTPACT = false;
volatile bool flagDOWNLOAD = false;
volatile bool flagProcessing = false;

char GSN[]="000000000000000";

const int modem_size = 180;       // Size of Modem serila buffer
char modem_buffer[modem_size];    // Modem serial buffer 
char url_buffer[modem_size];    // Modem serial buffer 
char command_buffer[modem_size];    // Modem serial buffer 
char modem_char;                  // Modem serial char
volatile int modem_i;            // Modem serial index
volatile int edges[2];

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
char date[7];
char time[9];
char lat_indicator[2];
char lng_indicator[2];
char altitude[7]; 
char speed[7]; 
char hdop[7];
char timestamp[] = "1624031494587";
 

volatile float lat;
volatile float lng;

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
  float val = atof(coordinate);
  val = val / 100.0;
  if (indicator[0] == 'S' || indicator[0] == 'W') val = 0 - val;
  return val;
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
    "%s?id=%s&lat=%s&lon=%s&timestamp=%s&hdop=%s&altitude=%s&speed=%s",
    domain, GSN, latitude, longitude, timestamp, hdop, altitude, speed
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

void initHTTP(){
  sendCommand("HTTPINIT", 3);
}

void postHTTP(){
  create_command();
  sendCommand(command_buffer, 5);
  sendCommand("HTTPACTION=1", 15);
}

void stopHTTP(){
  sendCommand("HTTPTERM", 3);
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
          // if (memcmp(mCBC,   modem_buffer,4)==0) procCBC();
          // if (memcmp(mCSQ,   modem_buffer,4)==0) procCSQ();
          // if (memcmp(mCGR,   modem_buffer,4)==0) procCGR();
          if (memcmp(mCGN,   modem_buffer,4)==0) procCGN();
          if (memcmp(mGSN,   modem_buffer,4)==0) procGSN();
          // if (memcmp(mCOP,   modem_buffer,4)==0) procCOP();
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
    Serial.println(".... uart test");
    if (sendCommand("GSN", 3)) {
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
