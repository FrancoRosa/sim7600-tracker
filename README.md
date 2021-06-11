
## Devices:
### SIM7600
[Aliexpress](https://www.aliexpress.com/item/32864966695.html?trace=wwwdetail2mobilesitedetail)


## HTTP Commands Abstract

[Application note](https://simcom.ee/documents/SIM7000x/SIM7000%20Series_HTTP_Application%20Note_V1.01.pdf)

### Bearer Configure

Command|Response|Description
-----|-----|-----
AT+SAPBR=1,1|OK|To open bearer
AT+SAPBR=2,1|+SAPBR:1, 1, "10.89.193.1"|To query bearer
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
AT+CGPS=1,1 / AT+CGPS=1 / AT+CGPSCOLD / AT+CGPSHOT|OK|Start GPS stand alone
AT+CGNSSINFO|+CGNSSINFO: 2,06,03,00,3426.693019,S,15051.184731,E,170521,034216.0,46.5,0.0,0.0,1.2,0.9,0.9|
|GNSS Data|

Value|Tag|Description
-----|-----|-----
2|<mode>| Fix mode 2=2D fix 3=3D fix
06|<GPS-SVs>| GPS satellite valid numbers scope: 00-12
03|<GLONASS-SVs>| GLONASS satellite valid numbers scope: 00-12
00|<BEIDOU-SVs>| BEIDOU satellite valid numbers scope: 00-12
3426.693019|<lat>| Latitude of current position. Output format is ddmm.mmmmmm
S|<N/S>| N/S Indicator, N=north or S=south
15051.184731|<log>| Longitude of current position. Output format is dddmm.mmmmmm
E|<E/W>| E/W Indicator, E=east or W=west
170521|<date>| Date. Output format is ddmmyy
034216.0|<UTC-time>| UTC Time. Output format is hhmmss.s
46.5|<alt>| MSL Altitude. Unit is meters.
0.0|<speed>| Speed Over Ground. Unit is knots.
0.0|<course>| Course. Degrees.
1.2|<PDOP>| Position Dilution Of Precision.
0.9|<HDOP>| Horizontal Dilution Of Precision.
0.9|<VDOP>| Vertical Dilution Of Precision



## Usefull extra commands
[AT commands reference](http://mt-system.ru/sites/default/files/documents/sim7500_sim7600_series_at_command_manual_v2.00.pdf)
Command|Response|Description
-----|-----|-----
AT+CMEE=2 || Enable verbose error codes
AT+CGMM||Get modem model
AT+GSN||Get IMEI



AT+CGREG?|| Get Network status
AT+CGREG=1|| register to network

AT+CSQ||Network signal level
AT+CFUN=6||Reset device

## Hardware Design
[Hardware Design](https://simcom.ee/documents/SIM7600E/SIM7600%20Series%20Hardware%20Design_V1.03.pdf)

AT+HTTPPARA="URL","http://www.sim.com"