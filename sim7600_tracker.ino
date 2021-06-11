// - - - - - - - - - - - - - - - - - - - - - - - 
// Commands to compile/upload and monitor output
// arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// arduino-cli upload --port /dev/ttyACM0 --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
// picocom /dev/ttyACM0 
// - - - - - - - - - - - - - - - - - - - - - - - 

#define led_pin 13

const char domain[] = "iotnetwork.com.au:5055";

volatile bool flagOK = false;
volatile bool flagERROR = false;
volatile bool flagREG = false;
volatile bool flagGNS = false;
volatile bool flagHTTPACT = false;
volatile bool flagDOWNLOAD = false;

char GSN[15];

const int sw_z = 120; // Size of UARTBuffer
char sw_b[sw_z];
char sw_c;          //SoftSerial char

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
const char mGSN[] = "86";  // this is SIMCOMS id found on serial number
const char mRIN[] = "RING";  // when someones makes a call.. it should hang up
const char mHUP[] = "ATH";  // when someones makes a call.. it should hang up
// - - - - - - - - - - - - - - - - - - - - - - - 


void mPower() {
  Serial.println("... restarting modem");
}

// Read modem IMEI
void procGSN() 
{
  int n=0;
  int k=0;
  sw_b[15]=0;
  for(n=0;n<13;n=n+2)
  {
    GSN[k]=(sw_b[n]-48)*10+(sw_b[n+1]-48);
    k++;
  }
  GSN[7]=sw_b[14]-48;
}

bool sendCommand(const char *command,int timeout) {
  Serial1.print("AT+");
  Serial1.print(command);
  Serial1.print("\r");
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
      delay(100);
      return true;
    }
    delay(100);
  }
  mPower();
  return false;
}



void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(115200);
  Serial1.begin(115200);
}

// get imei
// get location
// make post to server

void loop() {
  char c;
  while(Serial1.available()) {
    c = Serial1.read();
    Serial.print(c);
  }
  delay(10000);
  Serial.println(".... uart test");
  sendCommand("GSN", 5);
  digitalWrite(led_pin, !digitalRead(led_pin));
}
