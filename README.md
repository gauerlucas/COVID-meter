## COVID-meter

This is a small Arduino-based project used to test the connection with NodeMCU V3 ESP8266 modules.

It displays COVID epidemic indicators on a LCD display, and flashes LEDs for each contamination, hospital death or discharge (flashing frequency is based on the mean time between each event based on daily count).

1.  The NodeMCU module is connected to Internet

2.  Data are obtained online by the CoronavirusAPI-France : <https://github.com/florianzemma/CoronavirusAPI-France>

3.  The JSON is sent as a text string to the Arduino UNO through Serial

4.  The JSON string is parsed to update epidemic indicators on the Arduino UNO.

The codes need to be loaded on each board while they are physically disconnected to prevent Serial transmission error.
