#include <ESP32Lib.h>
#include <Ressources/CodePage437_8x16.h>

CompositeL8 comp;
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
  comp.clear(comp.RGB(0, 0, 0));
  comp.rect(0, 0, 360, 270, 255);
  for(int i = 0; i < 10; i++)
  {
    if(i < 5)
    {
      player[i][0] = 100 + 40 * i;
      player[i][1] = 135 - 25; 
      player[i][2] = 0;
      player[i][3] = -1; 
    }
    else
    {
      player[i][0] = 100 + 40 * i - 200;
      player[i][1] = 135 + 25; 
      player[i][2] = 0;
      player[i][3] = 1;     
    }
    player[i][4] = -1;
    comp.dot(player[i][0], player[i][1], 254 - i * 10);
  }
  comp.setCursor(80, 125);
  comp.print("Get Ready and press [Start]");
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
      int pix = comp.get(player[i][0], player[i][1]);
      //Serial.print(pix);
      //Serial.print(" ");
      if(pix > 100)
      {
        player[i][4] = pix;
      }
      else
      {
        comp.dot(player[i][0], player[i][1], 254 - i * 10);
        living++;
        lastLiving = i;
      }
    }
    if(living <= 1)
    {
      ended = true;
      winner = lastLiving;
      comp.setCursor(30, 125); 
      if(winner == -1)
      {
        comp.setCursor(120, 125); 
        comp.print("Draw. Press [Start]"); 
      }
      else
      {
        comp.setCursor(30, 125); 
        comp.print("Player ");
        comp.print(winner);
        comp.print(". You are winner! Press [Start]"); 
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
  //comp.setFrameBufferCount(2);
  comp.init(comp.MODE400x300, comp.XPlayer);
  comp.MODE400x300.print(Serial);
  //selecting the font
  comp.setFont(CodePage437_8x16);
  comp.setTextColor(comp.RGB(255, 255, 255), comp.RGB(0, 0, 0));  
  /*
  comp.setTextColor(comp.RGB(255, 255, 0), comp.RGB(0, 255, 255));
  comp.println("CodePage437_8x16");
  for (int i = 0; i < 256; i++)
    comp.print((char)i);
  comp.println();*/
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
        comp.fillRect(10, 120, 340, 30, comp.RGB(0, 0, 0));
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
  //comp.rect(0, 0, 360, 270, 255);
  //comp.show();
}
