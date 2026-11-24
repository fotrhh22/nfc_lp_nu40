#pragma once
#include "SPI.h"
void hostPresent(uint16_t*,uint16_t,uint16_t);
struct Adafruit_ILI9341{
 Adafruit_ILI9341(MockSPI*,int dc,int cs,int rst){assert(dc==3&&cs==44&&rst==4);}
 void begin(uint32_t hz){assert(hz==32000000);}
 void setRotation(int r){assert(r==1);}
 void drawRGBBitmap(int x,int y,uint16_t* p,uint16_t w,uint16_t h){assert(x==0&&y==0);hostPresent(p,w,h);}
};
