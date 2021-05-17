

## HTTP Commands Abstract

[Application note](https://simcom.ee/documents/SIM7000x/SIM7000%20Series_HTTP_Application%20Note_V1.01.pdf)

### Bearer Configure

Command|Response|Description
-----|-----|-----
AT+SAPBR=3, 1, "APN", "CMNET"|OK|Configure bearer profile 1
AT+SAPBR=1, 1|OK|To open bearer
AT+SAPBR=2, 1|+SAPBR:1, 1, "10.89.193.1"|To query bearer
AT+SAPBR=0, 1|OK|To close bearer

### Get Method

Command|Response|Description
-----|-----|-----
AT+HTTPINIT|OK|Init HTTP service
AT+HTTPPARA="CID",1|OK|Set parameters for HTTP session
AT+HTTPPARA="URL","www.sim.com" |OK|Set parameters for HTTP session
AT+HTTPACTION=0 |OK / +HTTPACTION: 0,200,1000 |GET session start / GET successfully
AT+HTTPREAD|+HTTPREAD: 1000 / OK| Read the data of HTTP server
AT+HTTPTERM|OK|Terminate HTTP service

## Glonass Commands Abstract
[Application Note](https://microchip.ua/simcom/LTE/SIM7500_SIM7600/Application%20Notes/SIM7500_SIM7600%20Series_GNSS_Application%20Note_V2.00.pdf)

Command|Response|Description
-----|-----|-----
||
||
||

AT+CGPS=1,1 (or AT+CGPS=1)
AT+CGPSCOLD
AT+CGPSHOT

## Usefull extra commands
[AT commands reference](https://simcom.ee/documents/SIM7000E/SIM7000%20Series_AT%20Command%20Manual_V1.03.pdf)
Command|Response|Description
-----|-----|-----
AT+CMEE=2 ||
||
||

AT+CGNSSMODE?
AT+CGNSSMODE=?

AT+CGPSINFO
AT+CGNSCOLD
AT+CGNSPWR=? 