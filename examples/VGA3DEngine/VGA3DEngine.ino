//This example displays a 3D model on a VGA screen. Double buffering is used to avoid flickering.
//You need to connect a VGA screen cable and an external DAC (simple R2R does the job) to the pins specified below.
//cc by-sa 4.0 license
//bitluni

//include libraries
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//include model
#include "thinker.h"
Mesh<VGA14Bit> model(thinker::vertexCount, thinker::vertices, 0, 0, thinker::triangleCount, thinker::triangles, thinker::triangleNormals);

//pin configuration
const int redPins[] = {2, 4, 12, 13, 14};
const int greenPins[] = {15, 16, 17, 18, 19};
const int bluePins[] = {21, 22, 23, 27};
const int hsyncPin = 32;
const int vsyncPin = 33;

//VGA Device
VGA14Bit vga;
//3D engine
Engine3D<VGA14Bit> engine(1337);

//initial setup
void setup()
{
	//need double buffering
	vga.setFrameBufferCount(2);
	//initializing i2s vga
	vga.init(vga.MODE200x150, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
	//setting the font
	vga.setFont(Font6x8);
}

///a colorful triangle shader actually calculated per triangle
VGA14Bit::Color myTriangleShader(int trinangleNo, short *v0, short *v1, short *v2, const signed char *normal, VGA14Bit::Color color)
{
	//normals packed in 1 signed byte per axis
	const float scaleN = 1.0f / 127.0f;
	const float nx = normal[0] * scaleN;
	const float ny = normal[1] * scaleN;
	const float nz = normal[2] * scaleN;
	//return R5G5B4 color each normal axis controls each color component
	return (int(15 * nx + 16)) | (int(15 * nz + 16) << 5) | (int(7 * ny + 8) << 10);
}

//render 3d model
void drawModel()
{
	//perspective transformation
	static Matrix perspective = Matrix::translation(vga.xres / 2, vga.yres / 2, 0) * Matrix::scaling(100 * vga.pixelAspect(), 100, 100) * Matrix::perspective(90, 1, 10);
	static float u = 0;
	u += 0.02;
	//rotate model
	Matrix rotation = Matrix::rotation(-1.7, 1, 0, 0) * Matrix::rotation(u, 0, 0, 1);
	Matrix m0 = perspective * Matrix::translation(0, 1.7 * 0, 7) * rotation * Matrix::scaling(7);
	//transform the vertices and normals
	model.transform(m0, rotation);
	//begin adding triangles to render pipeline
	engine.begin();
	//add this model to the render pipeline. it will sort the triangles from back to front and remove backfaced. The tiangle shader will determine the color of the tirangle.
	//the RGB color gien in the second parameter is not used in this case but could be used for calculations in the triangle shader
	model.drawTriangles(engine, vga.RGB(128, 70, 20), myTriangleShader);
	//render all triangles in the pipeline. if you render multiple models you want to do this once at the end
	engine.end(vga);
}

//just draw each frame
void loop()
{
	//calculate the milliseconds passed from last pass
	static int lastMillis = 0;
	int t = millis();
	//calculate fps (smooth)
	static float oldFps = 0;
	float fps = oldFps * 0.9f + 100.f / (t - lastMillis);
	oldFps = fps;
	lastMillis = t;
	//clear the back buffer
	vga.clear(0);
	//draw the model
	drawModel();
	//reset the text cursor
	vga.setCursor(0, 0);
	//print the stats
	vga.print("fps: ");
	vga.print(fps, 1, 4);
	vga.print(" tris/s: ");
	vga.print(int(fps * model.triangleCount));
	vga.show();
}