// Helper microcontroller to reset the featherM0 when required
// Open a terminal and follow the instructions
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  arduino-cli compile --fqbn arduino:avr:nano featherReset
//  arduino-cli upload --port /dev/ttyUSB0 --fqbn arduino:avr:nano featherReset
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define reset_pin A0 
#define reset LOW

void promt() {
  Serial.println("");
  Serial.println("...............................");
  Serial.println(".        Feather helper       .");
  Serial.println(".   Press 'r' to restart      .");
  Serial.println(".   Press 'u' to upload mode  .");
  Serial.println("...............................");
  Serial.print(" > ");
}

void setup() {
  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, !reset);
  Serial.begin(9600);
  promt();
}

void upload_feather(){
  digitalWrite(reset_pin, reset);
  delay(300);
  Serial.print(".");
  digitalWrite(reset_pin, !reset);
  delay(300);
  Serial.print(".");
  digitalWrite(reset_pin, reset);
  delay(300);
  Serial.print(".");
  digitalWrite(reset_pin, !reset);
  Serial.println(" upload mode");
  promt();
}

void reset_feather(){
  digitalWrite(reset_pin, reset);
  delay(300);
  Serial.print(".");
  digitalWrite(reset_pin, !reset);
  delay(300);
  Serial.print(".");
  delay(300);
  Serial.print(".");
  delay(300);
  Serial.println(" feather restarted");
  promt();
}

char c;
void loop() {
  c = Serial.read();
  if(c == 'u') upload_feather();
  if(c == 'r') reset_feather();
  delay(10);
}
