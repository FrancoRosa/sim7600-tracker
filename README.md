# Sim7600 Tracker

> This project uses a Feather M0 microcontroller and a SIMCOM7600 LTE Glonass Module as a tracking device

- [Sim7600 Tracker](#sim7600-tracker)
  - [SMS Config](#sms-config)
  - [Devices](#devices)
    - [SIM7600](#sim7600)
  - [HTTP Commands Abstract](#http-commands-abstract)
    - [Bearer Configure](#bearer-configure)
    - [HTTP Request](#http-request)
  - [Glonass Commands Abstract](#glonass-commands-abstract)
  - [SMS Commands Abstract](#sms-commands-abstract)
  - [Power save mode](#power-save-mode)
  - [Usefull extra commands](#usefull-extra-commands)
  - [Hardware Design](#hardware-design)

## SMS Config

One of the features of this project is that you can set configuration parameters on the go by SMS
Sample message|Description
-----|-----
#*, server, http://iotnetwork.com.au:5055/,|Server and port configuration
#*, stationary, 300, |Period in seconds to save location if speed < 1
#*, logging, 20, |Period in seconds to save location if speed >= 1
#*, upload, 300, |Period in seconds to upload location saved
#*, recovery, 1, |When set recovery to 1, the device will send its location to server every 10 segs
#*, recovery, 0, |Disable revocery mode and return to normal working code
#*, sms, +00987654321, |Send SMS from tracker to especified number with location data, include country code(Ex. +00)

The commands are character sensible so dont include blank spaces and dont forget to type all the *", "* (commas) shown on the sample message

## Devices

### SIM7600
Modem with LTE and GPS/Glonass/Beidou features [Aliexpress](https://www.aliexpress.com/item/32864966695.html?trace=wwwdetail2mobilesitedetail)

## HTTP Commands Abstract

The commands provided by SIMCOM can be found here [Application note](https://simcom.ee/documents/SIM7000x/SIM7000%20Series_HTTP_Application%20Note_V1.01.pdf)

### Bearer Configure

This are the commands required for TCP/IP connections, this is not required for this modem model
Command|Response|Description
-----|-----|-----
AT+SAPBR=1, 1|OK|To open bearer
AT+SAPBR=2, 1|+SAPBR:1, 1, "10.89.193.1"|To query bearer
AT+SAPBR=0, 1|OK|To close bearer

### HTTP Request

You can make a POST/GET request with the followin set of commands

Command|Response|Description
-----|-----|-----
AT+HTTPINIT|OK|Init HTTP service
AT+HTTPPARA="CID", 1|OK|Set parameters for HTTP session
AT+HTTPPARA="URL", "www.sim.com" |OK|Set parameters for HTTP session
AT+HTTPACTION=0 |OK / +HTTPACTION: 0, 200, 1000 |GET session start / GET successfully
AT+HTTPREAD|+HTTPREAD: 1000 / OK| Read the data of HTTP server
AT+HTTPTERM|OK|Terminate HTTP service

## Glonass Commands Abstract

On this especific module the GPS features are already enabled from boot, you can get the related documentation here [Application Note](https://microchip.ua/simcom/LTE/SIM7500_SIM7600/Application%20Notes/SIM7500_SIM7600%20Series_GNSS_Application%20Note_V2.00.pdf)

Command|Response|Description
-----|-----|-----
AT+CGPS=1, 1 / AT+CGPS=1 / AT+CGPSCOLD / AT+CGPSHOT|OK|Start GPS stand alone
AT+CGNSSINFO|+CGNSSINFO: 2, 06, 03, 00, 3426.693019, S, 15051.184731, E, 170521, 034216.0, 46.5, 0.0, 0.0, 1.2, 0.9, 0.9|
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

## SMS Commands Abstract

In order to get SMS we need to use the following set of commands, [AT commands reference](http://mt-system.ru/sites/default/files/documents/sim7500_sim7600_series_at_command_manual_v2.00.pdf)

Command|Response|Description
-----|-----|-----
AT+CSMS=0|OK|Select SMS service
AT+CPMS="ME", "ME", "ME"|OK|Select SMS storage
AT+CMGF=1|OK|Select SMS text format
AT+CNMI=2, 2|OK|Show SMS notification when it comes
+CMT: "+61419847400", "", "21/06/21, 13:23:10+40"||SMS Notification, location, index
Testing SMS commands||SMS Notification, location, index
AT+CMGRD=0|OK|Read and Delete SMS
AT+CNMA|OK|Send ACK to SMS sender
AT+CGNSSINFO |(*) +CGNSSINFO: 2... |  GNSS Data

(*) +CGNSSINFO: 2, 06, 03, 00, 3426.693019, S, 15051.184731, E, 170521, 034216.0, 46.5, 0.0, 0.0, 1.2, 0.9, 0.9  

## Power save mode

To disable RF features when they are not required we can use the following set of codes [SIM7600 Series PCIE Hardware Design](https://microchip.ua/simcom/LTE/SIM7500_SIM7600/SIM7600_Series_PCIE_Hardware_Design_V1.03.pdf)

Command|Response|Description
-----|-----|-----
AT+CFUN=1 | OK | Enable RF
AT+CFUN=0 | OK | Disable RF

## Usefull extra commands

The full list of commands can be found here: [AT commands reference](http://mt-system.ru/sites/default/files/documents/sim7500_sim7600_series_at_command_manual_v2.00.pdf) some additional commands used on this project are descrived bellow

Command|Response|Description
-----|-----|-----
AT+CMEE=2 | OK | Enable verbose error codes
AT+CGMM|OK |Get modem model
AT+GSN|OK |Get IMEI
AT+CGREG?|OK| Get Network status
AT+CGREG=1|OK| register to network
AT+CSQ|OK|Network signal level
AT+CFUN=6|OK|Reset device

## Hardware Design
More information about the modem can be found here: [Hardware Design](https://simcom.ee/documents/SIM7600E/SIM7600%20Series%20Hardware%20Design_V1.03.pdf)
