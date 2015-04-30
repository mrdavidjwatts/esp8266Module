# esp8266Module
Arduino code for the ESP8226 Module running the 9.2.2 firmware at 9600bps

It is a pretty simple setup and will stream line the process of sending and recieving data. This example sends a value and recieves the current UK time and weather. You can change the weather location too to get it for your city/country.

It should work on 328 based boards or better dues to the memory requirements.

Planned updates:

- (Half way there) Functions to show the avaiable wifi list and ability to choose by number and enter a password
- (Half way there, currently per function) Replacing the delays with a milis timeout // I am not sure how long this will take or even if it will be reliable
- (On it like a bonnet) Replacing switch statement to use ENUM to make the wifiStates more readable
- Suggestions?
