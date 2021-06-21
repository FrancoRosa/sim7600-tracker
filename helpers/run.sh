echo ">>> compile"
arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
echo ">>> upload"
arduino-cli upload --port /dev/ttyACM0 --fqbn adafruit:samd:adafruit_feather_m0 sim7600_tracker
echo ">>> run"
sleep 2
picocom /dev/ttyACM0 -b 230400
echo ">>> end"
