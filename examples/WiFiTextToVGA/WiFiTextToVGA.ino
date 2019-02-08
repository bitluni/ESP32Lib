#include <stdio.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

//AP credentials
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
	vga.println(server.args());
}

void setup()
{
	WiFi.softAP(ssid, password, 6, 0);
	server.on("/", sendPage);
	server.on("/text", text);
	server.begin();
	//initializing i2s vga and frame buffers
	vga.init(vga.MODE360x400, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	vga.clear();
	vga.setFont(Font6x8);
	vga.println("----------------------");
	vga.println("bitluni's VGA Terminal");
	vga.print("Try SSID: ");
	vga.println(ssid);
	vga.println("----------------------");
}

void loop()
{
	server.handleClient();
	delay(10);
}