#http://demo.traccar.org:5055/?id=123456&lat={0}&lon={1}&timestamp={2}&hdop={3}&altitude={4}&speed={5}
from time import time

# user = admin
# password = changethispassword

import requests
domain = "iotnetwork.com.au"
port = 5055
id = 863922031635619
latitude = "-13.20416"
longitude = "-72.20898"
timestamp = int(time())
hdop = 12
altitude = 3400
speed = 10
url = "http://%s:%d/?id=%d&lat=%s&lon=%s&timestamp=%d&hdop=%d&altitude=%d&speed=%d"%(
  domain,port,id,latitude,longitude,timestamp,hdop,altitude,speed
)
 
response = requests.post(url)
print(response)

