//Arduino code for the ESP8226 Module running the 9.2.2 firmware at 9600bps by David Watts
//youtube.com/mrdavidjwatts
#include <EEPROM.h>
#include "U8glib.h" // v.1.17
#include "SoftwareSerial.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);	// Display which does not send AC
SoftwareSerial esp8266Module(10, 11); // RX, TX
// Misc variables
int statusLED = 13;
// Store network credentials variables
int epWifiStart;
int epPasswordStart;
// Inputting text variables
int colNum = 1;
int rowNum = 1;
char selectedChar;
char allChars[4][26] = {
	{'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'},
	{'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'},
	{32,'!','"','#','$','%','&','\'','(',')','*','+','`','-','_','.','\\','/',']','[','^','}','{','|','~',':'},
	{'0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5'}
};
int cursorPos = 1;
String cursor = "  <";
// Display only variables
char* mainTitles[] = {"ESP8266","Connect to Wifi", "Find Networks", "Run"};
char* runningTitles[] = {"Running","Status:", "Network:","Time:", "Date:", "Loops:", "Temperature"};
// Output variables
int loopNum = 0; // how many successful requests have we made?
String runningValues[7] = {"","","","","","",""};
String RecData; // Data returned
String ipAddress;
// Program flow variables
bool connected = false;
bool breakOut = false;
enum States {
	main,
	connect,
	findWifi,
	selectWifi,
	enterPassword,
	run
};
States state = main;
int runningFunctionNow = 0;
int wifiStatus = 1; // tracking the module status
// Finding networks variables
String password; // important
String network; // important
String DiscoveredNetworks[15] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
int selectedNetwork = 0;
String tempWifi;
int wifiNumber = 0;
int foundNetworks = 0;
bool foundWifi  = false;
unsigned long functionStartTime; //used to store the time a function started
const unsigned long timeout = 8000; //the timeout/maximum time in milliseconds

void setup() {
	delay(5000);
	Serial.begin(9600);
	esp8266Module.begin(9600); // Starting up the Serial for the ESP8266
	delay(5000);
	pinMode(statusLED, OUTPUT);
	pinMode(7, INPUT); // Getting the input buttons ready
	pinMode(6, INPUT);
	pinMode(5, INPUT);
	pinMode(4, INPUT);
	// Finding out if we have stored network credentials in EEPROM
	if(storedNetworkDetails()){
		readEPNetworkAndPassword();
	}
}

void loop() {
	// Start Display Stuff
	if(breakOut == false){
		u8g.firstPage();
		do {
			displayShow();
		} while ( u8g.nextPage() );
	}
	// End Display Stuff
	
	// Running the menu
	menu();
	
	// if we selected run then let the ESP8266 fly
	if (state == run)
	{
		runEsp8266("www.davidjwatts.com", "/arduino/esp8266.php");
	}
	// end flying ESP8266
}

// Our display function, display and font dependant, feel free to mess around here
void displayShow() {
	int posText = 23;
	switch (state) {
		case main:
		u8g.setFont(u8g_font_8x13B);
		u8g.setPrintPos(0, 13);
		u8g.print(mainTitles[0]);
		for(int i = 1; i < 4; i++){
			u8g.setFont(u8g_font_5x8);
			u8g.setPrintPos(0, posText);
			if(cursorPos == i){
				u8g.print(mainTitles[i] + cursor);
			}
			else
			{
				u8g.print(mainTitles[i]);
			}
			posText = posText + 8;
		}
		break;
		case connect:
		u8g.setFont(u8g_font_8x13B);
		u8g.setPrintPos(0, 13);
		u8g.print("Connect to Wifi");
		u8g.setFont(u8g_font_5x8);
		u8g.setPrintPos(0, 26);
		if(network == ""){
			u8g.print("No stored credentials");
		}
		else
		{
			if(password == ""){
				u8g.print("No stored credentials");
			}
			else
			{
				u8g.print("Connect to?");
				u8g.setPrintPos(0, 36);
				u8g.print(network);
			}
		}
		break;
		case findWifi:
		u8g.setFont(u8g_font_8x13B);
		u8g.setPrintPos(0, 13);
		u8g.print("Finding Networks");
		u8g.setFont(u8g_font_5x8);
		u8g.setPrintPos(0, 26);
		u8g.print("Be patient...");
		break;
		case selectWifi:
		u8g.setFont(u8g_font_8x13B);
		u8g.setPrintPos(0, 13);
		u8g.print("Select Network");
		u8g.setFont(u8g_font_5x8);
		//////up///////// top    left    right
		u8g.drawTriangle(64,20, 58,26, 70,26);
		u8g.setPrintPos(0, 36);
		u8g.print(DiscoveredNetworks[cursorPos]);
		//////down/////// top    left    right
		u8g.drawTriangle(64,50, 58,44, 70,44);
		break;
		case enterPassword:
		u8g.setFont(u8g_font_5x8);
		u8g.setPrintPos(0, 10);
		u8g.print(network);
		u8g.setFont(u8g_font_8x13B);
		u8g.setPrintPos(60, 30);
		u8g.print(selectedChar);
		u8g.setFont(u8g_font_5x8);
		u8g.setPrintPos(0, 50);
		u8g.print(password);
		break;
		case run:
		runningValues[2] = network;
		runningValues[3] = splitToVal(RecData, "+", "@"); 
		runningValues[4] = splitToVal(RecData, "@", "|");
		runningValues[5] = String(loopNum);		
		runningValues[6] = splitToVal(RecData, "$", "^");		
		//WeatherVal = splitToVal(RecData, "^", "~");
		u8g.setFont(u8g_font_8x13B);
		u8g.setPrintPos(0, 13);
		u8g.print(runningTitles[0]);
		for(int i = 1; i < 6; i++){
			u8g.setFont(u8g_font_5x8);
			u8g.setPrintPos(0, posText);
			u8g.print(runningTitles[i]);
			String valuetemp = runningTitles[i];
			u8g.setPrintPos((valuetemp.length() * 5) + 5, posText);
			u8g.print(runningValues[i]);
			posText = posText + 8;
		}
		
		break;
	}
	
	
}

// Menu function for selecting modes and states
void menu(){
	switch (state) {
		case main:
		wifiStatus = 0;
		if(digitalRead(7) == HIGH){
			if(cursorPos < 3){
				cursorPos++;
			}
			else{
				cursorPos = 1;
			}
		}
		if(digitalRead(5) == HIGH){
			switch(cursorPos) {
				case 1:
				state=connect;
				break;
				case 2:
				state=findWifi;
				break;
				case 3:
				state=run;
				break;
			}
			
		}
		break;
		case connect:
		if(digitalRead(5) == HIGH){
			if(storedNetworkDetails() && connected == false){
				readEPNetworkAndPassword();
			}
			
			if (network != "" && connected == false)
			{
				
			if (password == ""){
			state = enterPassword;	
			}
			else
			{
			connectToWifi();
			}
			}
			
			}

		
		if(digitalRead(4) == HIGH){
			state = main;
			
		}
		break;
		case findWifi:
		if(runningFunctionNow == 0){
			functionStartTime = millis();
			foundNetworks = 0;
			findWifiNetworks();
		}
		else
		{
			findWifiNetworks();
		}
		
		if(digitalRead(4) == HIGH){
			state = main;
		}
		break;
		case selectWifi:
		if(digitalRead(7) == HIGH){
			if(cursorPos < foundNetworks-1){
				cursorPos++;
			}
			else{
				cursorPos = 0;
			}
		}
		if(digitalRead(4) == HIGH){
			state = main;
		}
		if(digitalRead(5) == HIGH){
			network = DiscoveredNetworks[cursorPos];
			state = enterPassword;
		}
		break;
		case enterPassword:
		selectedChar = allChars[rowNum][colNum];
		if(digitalRead(7) == HIGH){
			if(rowNum < 3){
				rowNum++;
			}
			else{
				rowNum = 0;
			}
		}
		if(digitalRead(6) == HIGH){
			
			if(colNum < 25){
				colNum++;
			}
			else{
				colNum = 0;
			}
		}
		if(digitalRead(5) == HIGH){
			password += selectedChar;
		}
		if(digitalRead(4) == HIGH){
			state = main;
		}
		break;
		case run:
		if(digitalRead(4) == HIGH){
			state = main;
		}
		break;
        default:
      // statements
        break;

	}
}

// START WRITE WIFI DETAILS TO EEPROM
bool writeEPNetworkAndPassword(String inputNetwork, String inputPassword){
	
	int lengthNetwork = inputNetwork.length();
	int lengthPassword = inputPassword.length();
	int nextAddress = 1;
	
	EEPROM.write(nextAddress, lengthNetwork);
	nextAddress++;
	for(int i = 0; i < lengthNetwork; i++){

		char networkChar = inputNetwork.charAt(i);
		EEPROM.write(nextAddress, networkChar);
		nextAddress++;
	}

	EEPROM.write(nextAddress, lengthPassword);
	nextAddress++;
	for(int i = 0; i < lengthPassword; i++){

		char passwordChar = inputPassword.charAt(i);
		EEPROM.write(nextAddress, passwordChar);
		nextAddress++;
	}
	EEPROM.write(0, 1);
	
	if(EEPROM.read(0) == 1){
		return true;
	}
	else {
		return false;
	}
	
}
// END WRITE WIFI DETAILS TO EEPROM

// START STORED DETAILS CHECK
bool storedNetworkDetails(){
	if(EEPROM.read(0) == 1){
		return true;
	}
	else {
		return false;
	}
}
// END STORED DETAILS CHECK

// START READ DETAILS FROM EEPROM
bool readEPNetworkAndPassword(){
	network = "";
	password = "";
	int lengthNetwork = EEPROM.read(1);
	int nextAddress = 2;
	for(int i = 0; i < lengthNetwork; i++){
		char networkChar = EEPROM.read(nextAddress);
		network += networkChar;
		nextAddress++;
	}
	int lengthPassword = EEPROM.read(nextAddress);
	nextAddress++;
	for(int i = 0; i < lengthPassword; i++){
		char passwordChar = EEPROM.read(nextAddress);
		password += passwordChar;
		nextAddress++;
	}
}
// END READ DETAILS FROM EEPROM

// START FIND WIFI NETWORKS
bool findWifiNetworks() {
	breakOut = true;
	
	char character;
	if (runningFunctionNow == 1 && wifiNumber < 16) {
		if (esp8266Module.available() > 0) {
			character = esp8266Module.read();
			if(!foundWifi){
				//Serial.write(temp);
				if (character == ':') {
					foundWifi = true;
				}
			}
			else{
				if (character == '\n') {
					DiscoveredNetworks[wifiNumber] = splitWifi(tempWifi);
					tempWifi = "";
					wifiNumber++;
					foundNetworks++;
					foundWifi = false;
				}
				else {
					tempWifi.concat(character);
				}
			}
		}

		
	}
	else {
		if (wifiNumber < 1 && runningFunctionNow == 0) {
			runningFunctionNow = 1;
			esp8266Module.println(F("AT+CWLAP"));
		}
	}
	if((millis() - functionStartTime) > timeout){
		if(wifiNumber > 0){
			runningFunctionNow = 0;
			breakOut = false;
			selectedNetwork = 0;
			functionStartTime = 0;
			password = "";
			network = "";
			wifiNumber = 0;
			foundWifi = false;
			tempWifi = "";
			digitalWrite(statusLED, LOW);
			state = selectWifi;
			return true;
		}
		else{
			return false;
		}
	}

}
// END FIND WIFI NETWORKS

// SPLIT UP STRINGS --- This is not ready
String splitWifi(String inputWifi) {
	int firstListItem = inputWifi.indexOf("\"");
	int secondListItem = inputWifi.indexOf("\"", firstListItem + 1 );
	return inputWifi.substring(firstListItem + 1, secondListItem);
}
// END SPLIT UP STRINGS

// MAIN ESP8266 FUNCTION
void runEsp8266(String website, String page) {
	// 0 need to reset or beginning of loop
	// 1 reset complete check wifi mode
	// 2 wifi mode is 3, now check network connection
	// 3 If not connected connect to network
	// 4 request page from server
	// 5 unlink from server after request
	// 6 close network connection
	switch (wifiStatus) {
		case 0:    // 0 need to reset or beginning of loop
		Serial.println(F("TRYING esp8266Reset"));
		esp8266Reset();
		break;
		case 1:    // 1 reset complete check wifi mode
		Serial.println(F("TRYING changeWifiMode"));
		changeWifiMode();
		break;
		case 2:    // 2 wifi mode is 3, now check network connection
		Serial.println(F("TRYING checkWifiStatus"));
		checkWifiStatus();
		break;
		case 3:    // 3 If not connected connect to network
		Serial.println(F("TRYING connectToWifi"));
		connectToWifi(); // use this one to only use the find networks method
		//connectToWifi("---", "---"); use this one to hardcode initial details
		break;
		case 4:    // 4 request page from server
		Serial.println(F("TRYING getPage"));
		getPage(website, page);
		//getPage(website, page, "?num=", "3", "&num2=", "2000");
		break;
		case 5:    // 5 unlink from server after request
		Serial.println(F("TRYING unlinkPage"));
		unlinkPage();
		break;
        default:
        // statements
		break;
	}
}
// END MAIN ESP8266 FUNCTION

// 0 - RESET
bool esp8266Reset() {
	esp8266Module.println(F("AT+RST"));
	delay(7000);
	if (esp8266Module.find("OK"))
	{
		runningValues[1] = F("-RESET-");
		wifiStatus = 1;
		return true;
	}
	else
	{
		runningValues[1] = F("-FAILED-");
		wifiStatus = 0;
		return false;
	}
}
// END RESET

// 1 - CHANGE MODE
bool changeWifiMode()
{
	esp8266Module.println(F("AT+CWMODE?"));
	delay(5000);
	if (esp8266Module.find("3"))
	{
		runningValues[1] = F("Wifi Mode is 3");
		wifiStatus = 2;
		return true;
	}
	else
	{
		esp8266Module.println(F("AT+CWMODE=3"));
		delay(5000);
		if (esp8266Module.find("no change") || esp8266Module.find("OK"))
		{
			runningValues[1]= F("Wifi Mode is 3");
			wifiStatus = 2;
			return true;
		}
		else
		{
			//val1 = F("Wifi Mode failed");
			wifiStatus = 0;
			return false;
		}
	}

}
// END CHANGE MODE

// 2 - CHECK WIFI NETWORK STATUS
bool checkWifiStatus() {
	esp8266Module.println(F("AT+CWJAP?"));
	delay(5000);
	if (esp8266Module.find(":")) {
		//Serial.println(F("WIFI NETWORK CONNECTED"));
		String tempor = esp8266Module.readStringUntil('\n');
		tempor.replace("\"", "");
		network = tempor;
		runningValues[1] = tempor;
		wifiStatus = 4;
		return true;
	}
	else
	{
		wifiStatus = 3;
		return false;
	}
}
// END CHECK WIFI NETWORK STATUS

// 3 - CONNECT TO WIFI
bool connectToWifi() {
	if (network == "" || password == "")
	{
		state = findWifi;
	}
	else
	{
		
		String cmd = F("AT+CWJAP=\"");
		cmd += network;
		cmd += F("\",\"");
		cmd += password;
		cmd += F("\"");
		esp8266Module.println(cmd);
		delay(5000);
		if (esp8266Module.find("OK"))
		{
			//Serial.println(F("CONNECTED TO WIFI"));

			runningValues[1] = F("CONNECTED TO WIFI");
			wifiStatus = 4;
			return true;
			if(state == connect){
			state = main;
		}
		}
		else
		{
			wifiStatus = 0;
			return false;
		}
	}
}
// optional function that accepts the network ID and password as variables
bool connectToWifi(String networkId, String networkPassword) {
	network = networkId;
	String cmd = F("AT+CWJAP=\"");
	cmd += networkId;
	cmd += F("\",\"");
	cmd += networkPassword;
	cmd += F("\"");
	esp8266Module.println(cmd);
	delay(5000);
	if (esp8266Module.find("OK"))
	{
		Serial.println(F("CONNECTED TO WIFI"));

		runningValues[1] = F("CONNECTED TO WIFI");
		wifiStatus = 4;
		return true;
		if(state == connect){
			state = main;
		}
	}
	else
	{
		wifiStatus = 0;
		return false;
	}
}
// END CONNECT TO WIFI NETWORK

// 4 - GET PAGE
bool getPage(String website, String page) {
	//runningValues[1] = "";
	//Serial.println(website);
	String cmd = F("AT+CIPSTART=\"TCP\",\"");
	cmd += website;
	cmd += F("\",80");
	esp8266Module.println(cmd);
	delay(5000);
	if (esp8266Module.find("Linked"))
	{
		Serial.print(F("Connected to server"));

	}
	cmd =  "GET ";
	cmd += page;
	cmd += "?num=";  //construct the http GET request
	cmd += "1";
	cmd += "&weather=";
	cmd += "2644668";
	cmd += " HTTP/1.0\r\n";
	cmd += "Host:";
	cmd += website;
	cmd += "\r\n\r\n";
	Serial.println(cmd);
	esp8266Module.print("AT+CIPSEND=");
	esp8266Module.println(cmd.length());
	Serial.println(cmd.length());

	if (esp8266Module.find(">"))
	{
		Serial.println(F("found > prompt - issuing GET request"));
		esp8266Module.println(cmd);
	}
	else
	{
		wifiStatus = 5;
		Serial.println(F("No '>' prompt received after AT+CPISEND"));
		runningValues[1] = F("Failed request, retrying...");
		return false;
	}

	while (esp8266Module.available() > 0)
	{
		esp8266Module.read();
	}

	if (esp8266Module.find("*")) {
		String tempMsg = esp8266Module.readStringUntil('\n');
		RecData = tempMsg;
		Serial.println(tempMsg);
		wifiStatus = 5;
		return true;
	}
	else {
		wifiStatus = 5;
		return false;
	}

}

// optional getPage function that accepts user variables
bool getPage(String website, String page, String urlVariableName1, String variable1, String urlVariableName2, String variable2) {
	String cmd = F("AT+CIPSTART=\"TCP\",\"");
	cmd += website;
	cmd += F("\",80");
	esp8266Module.println(cmd);
	delay(5000);
	if (esp8266Module.find("Linked"))
	{
		//Serial.print(F("Connected to server"));

	}
	cmd =  "GET ";
	cmd += page;
	cmd += urlVariableName1;  // something like ?num=
	cmd += variable1;
	cmd += urlVariableName2;  // something like &num2=
	cmd += variable2;
	cmd += " HTTP/1.0\r\n";
	cmd += "Host:";
	cmd += website;
	cmd += "\r\n\r\n";
	//Serial.println(cmd);
	esp8266Module.print("AT+CIPSEND=");
	esp8266Module.println(cmd.length());
	//Serial.println(cmd.length());

	if (esp8266Module.find(">"))
	{
		//Serial.println(F("found > prompt - issuing GET request"));
		esp8266Module.println(cmd);
	}
	else
	{
		wifiStatus = 5;
		//Serial.println("No '>' prompt received after AT+CPISEND");
		runningValues[1] = F("Failed request, retrying...");
		return false;
	}

	while (esp8266Module.available() > 0)
	{
		esp8266Module.read();
	}

	if (esp8266Module.find("*")) {
  	String tempMsg = esp8266Module.readStringUntil('\n');
		RecData = tempMsg;
		Serial.println(tempMsg);
		wifiStatus = 5;
		return true;
	}
	else {
		wifiStatus = 5;
		return false;
	}

}
// END GET PAGE

// 5 - UNLINK
bool unlinkPage() {
	esp8266Module.println(F("AT+CIPCLOSE"));
	delay(5000);
	if (esp8266Module.find("Unlink"))
	{
		runningValues[1] = F("UNLINKED");
		wifiStatus = 0;
		loopNum++;
		return true;
	}
	else
	{
		wifiStatus = 4;
		return false;
	}
}
// END UNLINK

// 6 - CLOSE NETWORK  --- Probably ready...
bool closeNetwork() {
	esp8266Module.println(F("AT+CWQAP"));
	delay(5000);
	if (esp8266Module.find("OK"))
	{
		runningValues[1] = F("NETWORK DISCONNECTED");
		wifiStatus = 0;
		return true;
	}
	else
	{
		wifiStatus = 4;
		return false;
	}
}
// END CLOSE NETWORK

// SPLIT UP STRINGS
String splitToVal(String inputString, String delimiter, String endChar) {
	String tempString = "";
	int from = 0;
	int to = 0;
	for (int i = 0; i < inputString.length(); i++) {
		if (inputString.substring(i, i + 1) == delimiter) {
			from = i + 1;
		}
		if (inputString.substring(i, i + 1) == endChar) {
			to = i;
		}
	}
	tempString = inputString.substring(from, to);
	return tempString;
}
// END SPLIT UP STRINGS


// FLOAT TO STRING, IF YOU FANCY SENDING FLOATS OVER THE WEB

String floatToString(float inputFloat){
char CharBuffer[10];  
dtostrf(inputFloat,1,2,CharBuffer);
String floatString = String(CharBuffer);  
return floatString;
}
// END FLOAT TO STRING

// START FINDING IP ADDRESS
bool whatsMyIp() { 
	esp8266Module.println(F("AT+CIFSR"));
	delay(5000);
	if (esp8266Module.find("."))
	{
		ipAddress = esp8266Module.readStringUntil('\n');
	}
	else
	{
		return false;
	}
}
// END FINDING IP ADDRESS
