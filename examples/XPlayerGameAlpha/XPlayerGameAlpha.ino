#include <ESP32Lib.h>
#include <Ressources/CodePage437_8x16.h>

CompositeGrayLadder videodisplay;
GameControllers cont;
const int controllerPins[] = {33, 34, 35, 36, 39, 23, 18, 19, 5, 16};
//const int controllerPins[] = {36, 34};
const int clockPin = 2; //21
const int latchPin = 15; //4
//const int clockPin = 21;
//const int latchPin = 4;

int player[10][5];

bool started = false;
bool ended = false;
int winner = -1;

void initGame()
{
  videodisplay.clear(videodisplay.RGB(0, 0, 0));
  videodisplay.rect(0, 0, videodisplay.xres, videodisplay.yres, videodisplay.RGB(128, 128, 128));
  for(int i = 0; i < 10; i++)
  {
    if(i < 5)
    {
      player[i][0] = videodisplay.xres/2 + (videodisplay.xres/7) * (i) - (2*videodisplay.xres/7);
      player[i][1] = videodisplay.yres/2 - 25; 
      player[i][2] = 0;
      player[i][3] = -1; 
    }
    else
    {
      player[i][0] = videodisplay.xres/2 + (videodisplay.xres/7) * (i - 5) - (2*videodisplay.xres/7);
      player[i][1] = videodisplay.yres/2 + 25; 
      player[i][2] = 0;
      player[i][3] = 1;     
    }
    player[i][4] = -1;
    videodisplay.dot(player[i][0], player[i][1], 254 - i * 10);
  }
  videodisplay.setCursor(80, 125);
  videodisplay.print("Get Ready and press [Start]");
}


void processPlayers()
{
    int living = 0;
    int lastLiving = -1;
    for(int i = 0; i < 10; i++)
    {
      if(player[i][4] != -1) continue;
      if(cont.down(i, cont.UP) && player[i][3] == 0)
      {
        player[i][2] = 0;
        player[i][3] = -1;
      }
      else
      if(cont.down(i, cont.DOWN) && player[i][3] == 0)
      {
        player[i][2] = 0;
        player[i][3] = 1;
      }
      else
      if(cont.down(i, cont.LEFT) && player[i][2] == 0)
      {
        player[i][2] = -1;
        player[i][3] = 0;
      }
      else
      if(cont.down(i, cont.RIGHT) && player[i][2] == 0)
      {
        player[i][2] = 1;
        player[i][3] = 0;
      }
      player[i][0] += player[i][2];
      player[i][1] += player[i][3];
      int pix = videodisplay.get(player[i][0], player[i][1]);
      //Serial.print(pix);
      //Serial.print(" ");
      if(pix > 100)
      {
        player[i][4] = pix;
      }
      else
      {
        videodisplay.dot(player[i][0], player[i][1], 254 - i * 10);
        living++;
        lastLiving = i;
      }
    }
    if(living <= 1)
    {
      ended = true;
      winner = lastLiving;
      videodisplay.setCursor(30, 125); 
      if(winner == -1)
      {
        videodisplay.setCursor(120, 125); 
        videodisplay.print("Draw. Press [Start]"); 
      }
      else
      {
        videodisplay.setCursor(30, 125); 
        videodisplay.print("Player ");
        videodisplay.print(winner);
        videodisplay.print(". You are winner! Press [Start]"); 
      }
    }
    //Serial.println();
}

void setup() 
{
  Serial.begin(115200);
  cont.init(latchPin, clockPin);
  cont.setControllers(cont.SNES, 10, controllerPins);
  //we need double buffering for smooth animations
  //videodisplay.setFrameBufferCount(2);
  //videodisplay.init(CompMode::MODEPAL288P, videodisplay.XPlayer);
  //in the line below a modified MODEPAL288P is used, trimmed for overscan and centered
  videodisplay.init(ModeComposite(12+20, 38, 62+20, 400-40,  1+10,  6, 5, 5, 15+18, 288-28, 0, 0, 0, 0, 1, 8000000), videodisplay.XPlayer);
  //selecting the font
  videodisplay.setFont(CodePage437_8x16);
  videodisplay.setTextColor(videodisplay.RGB(255, 255, 255), videodisplay.RGB(0, 0, 0));  
  /*
  videodisplay.setTextColor(videodisplay.RGB(255, 255, 0), videodisplay.RGB(0, 255, 255));
  videodisplay.println("CodePage437_8x16");
  for (int i = 0; i < 256; i++)
    videodisplay.print((char)i);
  videodisplay.println();*/
  initGame();
}

void loop() 
{
  static int stepDelay = 50;
  cont.poll();
  if(ended)
  {
    for(int i = 0; i < 10; i++)
      if(cont.pressed(i, GameControllers::START))
      {
        started = false;
        ended = false;
        initGame();
        break;
      }
    return;
  }
  if(!started)
  {
    for(int i = 0; i < 10; i++)
      if(cont.pressed(i, GameControllers::START))
      {
        started = true;
        videodisplay.fillRect(10, 120, 340, 30, videodisplay.RGB(0, 0, 0));
        break;
      }
    return;
  }
  if(cont.pressed(0, GameControllers::L))
  {
    stepDelay = max(10, stepDelay - 10);
  }
  if(cont.pressed(0, GameControllers::R))
  {
    stepDelay = min(200, stepDelay + 10);    
  }
  static unsigned long t = 0;
  unsigned long dt = millis() - t;
  t += dt;
  static unsigned long stepT = 0;
  stepT += dt;
  if(stepT >= stepDelay)
  {
    stepT = 0;
    processPlayers();
  }
  //videodisplay.rect(0, 0, 360, 270, 255);
  //videodisplay.show();
}
