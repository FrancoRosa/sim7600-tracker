  // arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
  // arduino-cli upload --port /dev/ttyACM0 --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
  // picocom /dev/ttyACM0 

#define led_pin 13

void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(115200);
  Serial1.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
  char c;
  while(Serial1.available()) {
    c = Serial1.read();
    Serial.print(c);
  }
  delay(3000);
  Serial.println(".... uart test");
  Serial1.println("AT\r");
  digitalWrite(led_pin, !digitalRead(led_pin));
}
