#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#include <Wire.h>

#define DHTPIN 7  // Temperature/Humidity sensor
#define DHTTYPE DHT11

DHT dht_11(DHTPIN, DHTTYPE);

byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xA0, 0xA1 };
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "ICBRXCF794TQ1Z7V";
const int updateThingSpeakInterval = 3 * 1000;
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;
int potPin = 3;
int potValue = 0;
// Initialize Arduino Ethernet Client
EthernetClient client;
void setup() {
	Serial.begin(9600);
	dht_11.begin();
	startEthernet();
}

void loop() {
    potValue = analogRead(potPin); 
	String analogPin0 = String(analogRead(A0), DEC);
	if (client.available())	{
		char c = client.read();
		Serial.print(c);
	}

// DHT11
	char t_buffer[10];
	char h_buffer[10];
	float t = dht_11.readTemperature();
	String temp=dtostrf(t,0,5,t_buffer);
	float h = dht_11.readHumidity();
	String humid=dtostrf(h,0,5,h_buffer);


// Disconnect from ThingSpeak
    if (!client.connected() && lastConnected) {
    	Serial.println("...disconnected");
    	Serial.println();
    	client.stop();
    }

// Update ThingSpeak
    if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval)) {
    	updateThingSpeak("field1="+temp+"&field2="+humid+"&field3="+potValue);
    }

// Check if Arduino Ethernet needs to be restarted
    if (failedCounter > 3 ) 
    	{startEthernet();}
	lastConnected = client.connected();
}
    
void updateThingSpeak(String tsData) {
	if (client.connect(thingSpeakAddress, 80)) {
		client.print("POST /update HTTP/1.1\n");
		client.print("Host: api.thingspeak.com\n");
		client.print("Connection: close\n");
		client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
		client.print("Content-Type: application/x-www-form-urlencoded\n");
		client.print("Content-Length: ");
		client.print(tsData.length());
		client.print("\n\n");
		client.print(tsData);
		lastConnectionTime = millis();
		if (client.connected()) {
			Serial.println("Connecting to ThingSpeak...");
			Serial.println();
			failedCounter = 0;
		} else {
			failedCounter++;
			Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
			Serial.println();
		}
	} else {
		failedCounter++;
		Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
		Serial.println();
		lastConnectionTime = millis();
	}
}
void startEthernet() {
	client.stop();
	Serial.println("Connecting Arduino to network...");
	Serial.println();
	delay(1000);
	// Connect to network amd obtain an IP address using DHCP
	if (Ethernet.begin(mac) == 0) {
		Serial.println("DHCP Failed, reset Arduino to try again");
		Serial.println();
	} else {
		Serial.println("Arduino connected to network using DHCP");
		Serial.println();
	}
	delay(1000);
}
