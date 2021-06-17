// - - - - - - - - - - - - - - - - - - - - - - - 
// Commands to compile/upload and monitor output
// arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// arduino-cli upload --port /dev/ttyACM0 --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// picocom /dev/ttyACM0 
// - - - - - - - - - - - - - - - - - - - - - - - 

#include <FreeRTOS_SAMD21.h>

#define led_pin 13
#define key_pin 2
// #define Modem Serial1
// #define SerialUSB Serial

const char domain[] = "iotnetwork.com.au:5055";

volatile bool flagOK = false;
volatile bool flagERROR = false;
volatile bool flagREG = false;
volatile bool flagGNS = false;
volatile bool flagHTTPACT = false;
volatile bool flagDOWNLOAD = false;

char GSN[]="000000000000000";

const int modem_size = 120;       // Size of Modem serila buffer
char modem_buffer[modem_size];    // Modem serial buffer 
char modem_output[modem_size];    // Modem serial buffer 
char modem_char;                  // Modem serial char
volatile int modem_i;            // Modem serial index

// - - - - - - - - - - - - - - - - - - - - - - - 
const char mOK[]     = "OK";
const char mERROR[]  = "ERRO";
const char mCLOSED[] = "CLOS";
const char mSend[]   = "SEND OK";
const char mConn[]   = "CONNECT OK";
const char mCBC[] = "+CBC";
const char mCOP[] = "+COP";
const char mCSQ[] = "+CSQ";
const char mCGR[] = "+CGR";
const char mIPC[] = "+IPC";
const char mCGN[] = "+CGN";
const char mGSN[] = "86";   // this is SIMCOMS id found on serial number
const char mRIN[] = "RING"; // when someones makes a call.. it should hang up
const char mHUP[] = "ATH";  // when someones makes a call.. it should hang up
// - - - - - - - - - - - - - - - - - - - - - - - 

TaskHandle_t Handle_rxTask;
TaskHandle_t Handle_txTask;

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


void modemPower() {
  Serial.println("... restarting modem");
  digitalWrite(key_pin, LOW);
  osDelayMs(500);
  digitalWrite(key_pin, HIGH);
}

// Read modem IMEI
void procGSN() {
  Serial.println("... procesing GSN");
  memcpy(GSN, modem_buffer, 15);
  Serial.print("GSN: ");
  Serial.println(GSN);
}

void procCGN() {
  flagGNS = false;
  char *pch;
  pch = (char*) memchr(modem_buffer, ':', 10);
  if (pch != NULL) {
    if (modem_buffer[12]=='2' || modem_buffer[12]=='3') { // if GNS is fixed
      flagGNS = true;
      memmove(latitude , modem_buffer+33, 10); //JustPick lat and lon
      memmove(longitude, modem_buffer+44, 10); //JustPick lat and lon
      memmove(timestamp, modem_buffer+14, 18); //JustPick lat and lon
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

// get location
// make post to server

static void task_rx_modem(void *pvParameters) {
  osDelayS(1);
  while (true){
    while(Serial1.available()) {
      modem_char = Serial1.read();
      Serial.print(modem_char);
      modem_buffer[modem_i] = modem_char;
      modem_i++;
      if (modem_i >= modem_size) modem_i = 0;
      // if (modem_char=='>') flagPromt = true;
      if ((modem_i >= 2) && ((modem_char == '\n') || (modem_char == '\n')))
        {
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
          if (memcmp(mGSN,   modem_buffer,2)==0) procGSN();
          // if (memcmp(mCOP,   modem_buffer,4)==0) procCOP();
          modem_i=0;
          for(int i = 0; i < modem_size; i++) modem_buffer[i]=0; 
        }
    }
   osDelayMs(1);
  }
}

static void task_tx_modem(void *pvParameters) {
  osDelayS(1);
  while (true) {
    Serial.println(".... uart test");
    sendCommand("GSN", 5);osDelayS(1);
    // sendCommand("CGPS=1", 5);osDelayS(1);
    sendCommand("CGNSSINFO", 5);osDelayS(1);
    digitalWrite(led_pin, !digitalRead(led_pin));
    osDelayS(30);
  }
}

void setup() {
  delay(1000); // keep this to avoid USB crash
  pinMode(led_pin, OUTPUT);
  pinMode(key_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);
  digitalWrite(key_pin, HIGH);
  Serial.begin(115200);
  delay(1000); // keep this to avoid USB crash
  Serial1.begin(115200);
  delay(1000); // keep this to avoid USB crash
  
  xTaskCreate(task_tx_modem, "txModem", 256, NULL, tskIDLE_PRIORITY + 2, &Handle_txTask);
  xTaskCreate(task_rx_modem, "rxModem", 256, NULL, tskIDLE_PRIORITY + 1, &Handle_rxTask);
  vTaskStartScheduler();

  while(true) {
    Serial.println("OS Failed! \n");
    delay(1000);
  }
}

void loop() {
  delay(1000);
}
