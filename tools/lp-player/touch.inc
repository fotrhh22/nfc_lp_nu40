#ifndef LP_HOST_TEST
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#endif
struct TouchRecord;
struct TouchRaw {uint16_t x,y;bool down;};
struct TouchCalibration {int32_t left,right,top,bottom;bool swap;};
static TouchCalibration touchCal={};
static bool touchReady=false,calibrating=false,touchHeld=false,touchCandidate=false;
static uint8_t calibrationStep=0;
static TouchRaw corners[4];
static uint32_t lastTouchMs=0,touchChangedMs=0;
static void togglePlayback();static void changeVolume(int);static void renderAt(uint32_t);static void pushFinishedFrame();
static bool validTouchCalibration(){return abs(touchCal.right-touchCal.left)>500&&abs(touchCal.bottom-touchCal.top)>500;}
#ifdef LP_HOST_TEST
static TouchRaw hostRaw={0,0,false};
static TouchRaw readTouch(){return hostRaw;}
static bool loadTouchCalibration(){touchCal={200,3800,200,3800,false};return true;}
static void saveTouchCalibration(){}
#else
struct TouchRecord {uint32_t magic;uint16_t version,size;int32_t left,right,top,bottom;uint8_t swap,reserved[3];uint32_t checksum;};
static uint32_t touchChecksum(const TouchRecord& r){uint32_t h=2166136261u;const uint8_t* p=reinterpret_cast<const uint8_t*>(&r);for(size_t i=0;i<offsetof(TouchRecord,checksum);i++)h=(h^p[i])*16777619u;return h;}
static bool loadTouchCalibration(){
 if(!InternalFS.begin())return false;
 const char* paths[]={"/lp-touch.bin","/lyrics-touch.bin","/touch-calibration.bin"};
 for(const char* path:paths){using namespace Adafruit_LittleFS_Namespace;File f(path,FILE_O_READ,InternalFS);if(!f)continue;TouchRecord r={};const int n=f.read(&r,sizeof r);f.close();if(n!=sizeof r||r.magic!=0x46315443u||r.version!=1||r.size!=sizeof r||r.checksum!=touchChecksum(r))continue;touchCal={r.left,r.right,r.top,r.bottom,r.swap!=0};if(validTouchCalibration())return true;}
 return false;
}
static void saveTouchCalibration(){
 TouchRecord r={};r.magic=0x46315443u;r.version=1;r.size=sizeof r;r.left=touchCal.left;r.right=touchCal.right;r.top=touchCal.top;r.bottom=touchCal.bottom;r.swap=touchCal.swap;r.checksum=touchChecksum(r);
 if(InternalFS.exists("/lp-touch.bin"))InternalFS.remove("/lp-touch.bin");
 using namespace Adafruit_LittleFS_Namespace;File f("/lp-touch.bin",FILE_O_WRITE,InternalFS);if(f){f.write(reinterpret_cast<const uint8_t*>(&r),sizeof r);f.flush();f.close();}
}
static uint16_t medianTouch(uint16_t* a){for(int i=1;i<5;i++){uint16_t v=a[i];int j=i-1;while(j>=0&&a[j]>v){a[j+1]=a[j];j--;}a[j+1]=v;}return a[2];}
static TouchRaw readTouch(){
 uint16_t xs[5],ys[5],zs[5];digitalWrite(TFT_CS,HIGH);SPI.beginTransaction(SPISettings(2000000,MSBFIRST,SPI_MODE0));digitalWrite(TOUCH_CS,LOW);
 for(int i=0;i<5;i++){SPI.transfer(0xd0);xs[i]=SPI.transfer16(0)>>3;SPI.transfer(0x90);ys[i]=SPI.transfer16(0)>>3;SPI.transfer(0xb0);const uint16_t z1=SPI.transfer16(0)>>3;SPI.transfer(0xc0);const uint16_t z2=SPI.transfer16(0)>>3;zs[i]=uint16_t(min<uint32_t>(uint32_t(z1)+4095-z2,4095));}
 digitalWrite(TOUCH_CS,HIGH);SPI.endTransaction();const uint16_t z=medianTouch(zs),x=medianTouch(xs),y=medianTouch(ys);return {x,y,z>=200&&x>0&&x<4095&&y>0&&y<4095};
}
#endif
static void showCalibrationTarget(){
 memset(rgbFrame,0,sizeof rgbFrame);const int x=(calibrationStep==1||calibrationStep==2)?299:20,y=calibrationStep>=2?219:20;
 for(int d=-9;d<=9;d++){rgbFrame[y*320+x+d]=0xffff;rgbFrame[(y+d)*320+x]=0xffff;}pushFinishedFrame();
 Serial.println("Touch and release crosshair (TL, TR, BR, BL). Button 37 skips touch setup.");
}
static void finishTouchCalibration(){
 const int dx=abs(int(corners[1].x+corners[2].x)-int(corners[0].x+corners[3].x)),dy=abs(int(corners[2].y+corners[3].y)-int(corners[0].y+corners[1].y));
 const int sx=abs(int(corners[1].y+corners[2].y)-int(corners[0].y+corners[3].y)),sy=abs(int(corners[2].x+corners[3].x)-int(corners[0].x+corners[1].x));
 touchCal.swap=sx+sy>dx+dy;
 touchCal.left=touchCal.swap?(corners[0].y+corners[3].y)/2:(corners[0].x+corners[3].x)/2;
 touchCal.right=touchCal.swap?(corners[1].y+corners[2].y)/2:(corners[1].x+corners[2].x)/2;
 touchCal.top=touchCal.swap?(corners[0].x+corners[1].x)/2:(corners[0].y+corners[1].y)/2;
 touchCal.bottom=touchCal.swap?(corners[2].x+corners[3].x)/2:(corners[2].y+corners[3].y)/2;
 touchReady=validTouchCalibration();calibrating=!touchReady;
 if(touchReady){saveTouchCalibration();renderAt(0);}else{calibrationStep=0;showCalibrationTarget();}
}
static void touchDown(const TouchRaw& raw){
 if(calibrating){corners[calibrationStep]=raw;return;}
 if(!touchReady)return;
 const int32_t h=touchCal.swap?raw.y:raw.x,v=touchCal.swap?raw.x:raw.y;
 const int x=20+(h-touchCal.left)*279/(touchCal.right-touchCal.left),y=20+(v-touchCal.top)*199/(touchCal.bottom-touchCal.top);
 const int control=lpHitControl(x,y);
 if(control==1)togglePlayback();else if(control==-1)changeVolume(-1);else if(control==2)changeVolume(1);
}
static void pollTouch(uint32_t now){
 if(uint32_t(now-lastTouchMs)<20)return;lastTouchMs=now;const TouchRaw raw=readTouch();
 if(raw.down!=touchCandidate){touchCandidate=raw.down;touchChangedMs=now;}
 if(touchCandidate!=touchHeld&&uint32_t(now-touchChangedMs)>=30){
  touchHeld=touchCandidate;
  if(touchHeld)touchDown(raw);
  else if(calibrating){if(++calibrationStep==4)finishTouchCalibration();else showCalibrationTarget();}
 }
}
