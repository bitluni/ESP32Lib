//This example acts like a websever. It can create an access point or join an existing WiFI network.
//All text that's entered in the served page will bis displayed on the connected VGA screen.
//You need to connect a VGA screen cable to the pins specified below.
//cc by-sa 4.0 license
//bitluni

#include <stdio.h>
#include <WiFi.h>
#include <WebServer.h>
//ESP32Lib headers
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//true: creates an access point, false: connects to an existing wifi
const bool AccessPointMode = true;
//wifi credentials (enter yours if you arne not using the AccessPointMode)
const char *ssid = "VGA";
const char *password = "";

//pin configuration, change if you need to
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 32;
const int vsyncPin = 33;

//the webserver at pot 80
WebServer server(80);

//The VGA Device
VGA3Bit vga;

//include html page
const char *page =
#include "page.h"
	;

///Html page is sent on root
void sendPage()
{
	server.send(200, "text/html", page);
}

///Received text will be displayed on the screen
void text()
{
	server.send(200, "text/plain", "ok");
	vga.println(server.arg(0).c_str());
}

///initialization
void setup()
{
	Serial.begin(115200);
	//Handle the WiFi AP or STA mode and display results on the screen
	if (AccessPointMode)
	{
		Serial.println("Creating access point...");
		WiFi.softAP(ssid, password, 6, 0);
	}
	else
	{
		Serial.print("Connecting to SSID ");
		Serial.println(ssid);
		WiFi.begin(ssid, password);
		while (WiFi.status() != WL_CONNECTED)
		{
			delay(500);
			vga.print(".");
		}
	}
	//start vga on the specified pins
	vga.init(vga.MODE400x300, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	//make the background blue
	vga.clear(vga.RGBA(0, 0, 255));
	vga.backColor = vga.RGB(0, 0, 255);
	//select the font
	vga.setFont(Font6x8);

	//send page on http://<ip>/
	server.on("/", sendPage);
	//receive text on http://<ip>/text
	server.on("/text", text);
	//start the server
	server.begin();

	//display some text header on the screen including the ip
	vga.clear(vga.RGBA(0, 0, 255));
	vga.setCursor(0, 0);
	vga.println("----------------------");
	vga.println("bitluni's VGA Terminal");
	if (AccessPointMode)
	{
		vga.print("SSID: ");
		vga.println(ssid);
		if (strlen(password))
		{
			vga.print("password: ");
			vga.println(password);
		}
		vga.println("http://192.168.4.1");
	}
	else
	{
		vga.print("http://");
		vga.println(WiFi.localIP().toString().c_str());
	}
	vga.println("----------------------");
}

void loop()
{
	//process the server stuff
	server.handleClient();
	delay(10);
}