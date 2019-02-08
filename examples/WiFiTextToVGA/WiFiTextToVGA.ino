#include <stdio.h>
#include <WiFi.h>
#include <WebServer.h>

//true: creates an access point, false: connects to an existing wifi
const bool AccessPointMode = true;
//wifi credentials
const char *ssid = "VGA";
const char *password = "";

#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 0;
const int vsyncPin = 5;

WebServer server(80);
//VGA Device
VGA3Bit vga;

const char *page =
#include "page.h"
	;

void sendPage()
{
	server.send(200, "text/html", page);
}

void text()
{
	server.send(200, "text/plain", "ok");
	vga.println(server.arg(0).c_str());
}

void setup()
{
	Serial.begin(115200);
	if (AccessPointMode)
		WiFi.softAP(ssid, password, 6, 0);
	else
	{
		WiFi.begin(ssid, password);
		while (WiFi.status() != WL_CONNECTED)
		{
			delay(500);
			Serial.print(".");
		}
	}
	server.on("/", sendPage);
	server.on("/text", text);
	server.begin();

	//initializing i2s vga and frame buffers
	vga.init(vga.MODE360x400, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	vga.clear(4);
	vga.backColor = 4;
	vga.setFont(Font6x8);

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
	server.handleClient();
	delay(10);
}