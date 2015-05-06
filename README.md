# esp8266Module
Arduino code for the ESP8226 Module running the 9.2.2 firmware at 9600bps

It is a pretty simple setup and will stream line the process of sending and receiving data. This example sends a value and receives the current UK time and weather. You can change the weather location too to get it for your city/country.

- Can save network credentials to EEPROM
- Can read network details from EEPROM
- Can search for available WiFi networks and allow you to choose which to connect to
- Ability to enter password
- Program flow is controlled by readable ENUM values in switch statements
- Reliable and modifiable

It should work on 328 based boards or better dues to the memory requirements.

Planned and completed updates:

- (Finished) Functions to show the available WiFi list and ability to choose by number and enter a password
- (Half way there, currently per function) Replacing the delays with a milis timeout // I am not sure how long this will take or even if it will be reliable
- (Finished) Replacing switch statement to use ENUM to make the WiFi States more readable
- Suggestions?

The code currently comes in at - 
23,426 bytes
Global variables use 1,252 bytes of dynamic memory
