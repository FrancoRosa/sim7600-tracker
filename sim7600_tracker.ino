// - - - - - - - - - - - - - - - - - - - - - - - 
// Commands to compile/upload and monitor output
// arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// arduino-cli upload --port /dev/ttyACM0 --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// picocom /dev/ttyACM0 
// - - - - - - - - - - - - - - - - - - - - - - - 

#include <FreeRTOS_SAMD21.h>

#define led_pin 13
#define Modem Serial1
#define SerialUSB Serial

const char domain[] = "iotnetwork.com.au:5055";

volatile bool flagOK = false;
volatile bool flagERROR = false;
volatile bool flagREG = false;
volatile bool flagGNS = false;
volatile bool flagHTTPACT = false;
volatile bool flagDOWNLOAD = false;

char GSN[15];

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

volatile bool flagPromt = false;


TaskHandle_t Handle_rxTask;
TaskHandle_t Handle_txTask;

void osDelayUs(int us)
{
  vTaskDelay( us / portTICK_PERIOD_US );  
}

void osDelayMs(int ms)
{
  vTaskDelay( (ms * 1000) / portTICK_PERIOD_US );  
}

void osDelayS(int s)
{
  while (s>0){
    osDelayMs(1000);
    s--;    
  }  
}

void mPower() {
  SerialUSB.println("... restarting modem");
}

// Read modem IMEI
void procGSN() 
{
  int n=0;
  int k=0;
  modem_buffer[15]=0;
  for(n=0;n<13;n=n+2)
  {
    GSN[k]=(modem_buffer[n]-48)*10+(modem_buffer[n+1]-48);
    k++;
  }
  GSN[7]=modem_buffer[14]-48;
}

bool sendCommand(const char *command,int timeout) {
  Modem.print("AT+");
  Modem.print(command);
  Modem.print("\r");
  return waitOk(timeout);
}

bool waitOk(int timeout) {
  flagOK=0;
  flagERROR=0;
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
  mPower();
  return false;
}

// get location
// make post to server

static void task_rx_modem(void *pvParameters) {
  while (true){
    while(Modem.available()) {
      modem_char = Modem.read();
      SerialUSB.print(modem_char);
      modem_buffer[modem_i] = modem_char;
      modem_i++;
      if (modem_i >= modem_size) modem_i = 0;
      if (modem_char=='>') flagPromt = true;
      if ((modem_i>=2) && ((modem_char == '\n')|| (modem_char == '\n')))
        {
          modem_buffer[modem_i]='\0';
          if (memcmp(mOK,    modem_buffer,2)==0) flagOK=true;
          if (memcmp(mERROR, modem_buffer,4)==0) flagERROR=true;
          if (memcmp(mCLOSED,modem_buffer,4)==0) flagConn=false;
          if (memcmp(mSend,  modem_buffer,6)==0) flagSend=true;
          if (memcmp(mConn,  modem_buffer,9)==0) flagConn=true;
          if (memcmp(mRIN,   modem_buffer,4)==0) flagRIN=true;
          if (memcmp(mCBC,   modem_buffer,4)==0) procCBC();
          if (memcmp(mCSQ,   modem_buffer,4)==0) procCSQ();
          if (memcmp(mCGR,   modem_buffer,4)==0) procCGR();
          if (memcmp(mCGN,   modem_buffer,4)==0) procCGN();
          if (memcmp(mGSN,   modem_buffer,2)==0) procGSN();
          if (memcmp(mCOP,   modem_buffer,4)==0) procCOP();
          modem_i=0;
          for(i=0;i<modem_size;i++) modem_buffer[i]=0; 
        }
    }
   osDelayMs(1);
  }
}

static void task_tx_modem(void *pvParameters) {
  while (true) {
    osDelayS(10);
    SerialUSB.println(".... uart test");
    sendCommand("GSN", 5);
    digitalWrite(led_pin, !digitalRead(led_pin));
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);
  SerialUSB.begin(115200);
  delay(1000); // keep this to avoid USB crash
  Modem.begin(115200);
  
  xTaskCreate(task_tx_modem, "txModem", 256, NULL, tskIDLE_PRIORITY + 2, &Handle_txTask);
  xTaskCreate(task_rx_modem, "rxModem", 256, NULL, tskIDLE_PRIORITY + 1, &Handle_rxTask);
  vTaskStartScheduler();

  while(true) {
    SerialUSB.println("OS Failed! \n");
    delay(1000);
  }
}

void loop() {
  osDelayUs(100);
}
