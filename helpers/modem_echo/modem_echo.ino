
// Echo code to let the user write to the modem and see the result
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 modem_echo
// arduino-cli upload --port /dev/ttyACM0 --fqbn adafruit:samd:adafruit_feather_m0 modem_echo/
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

char c_modem;
char c_user;

void setup() {
  Serial.begin(230400);
  Serial1.begin(115200);
}

void loop() {
  if(Serial.available()) {
    c_user = Serial.read();
    Serial1.print(c_user);
  }
  if(Serial1.available()) {
    c_modem = Serial1.read();
    Serial.print(c_modem);
  }
}
