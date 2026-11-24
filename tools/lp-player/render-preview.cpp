#include "lp_assets.h"
static uint16_t pixels[320*240];
static bool saveFrame(const char* path){
 FILE* f=fopen(path,"wb");if(!f)return false;fprintf(f,"P6\n320 240\n255\n");
 for(uint16_t v:pixels){uint8_t rgb[3]={uint8_t(((v>>11)&31)*255/31),uint8_t(((v>>5)&63)*255/63),uint8_t((v&31)*255/31)};fwrite(rgb,1,3,f);}fclose(f);return true;
}
int main(int argc,char** argv){
 if(argc!=2)return 2;
 for(int mode=0;mode<4;mode++){
  LpState s;s.active=LP_TRACK_COUNT?0:-1;s.present=mode<2;s.playing=mode==0;s.manualPause=mode==1;s.unknown=mode==3;s.positionMs=mode<2?48500:0;
  if(mode==2)s.active=-1;
  lpRender(pixels,s,500);
  char path[1024];snprintf(path,sizeof path,"%s/frame-%d.ppm",argv[1],mode);if(!saveFrame(path))return 3;
 }
 for(unsigned frame=0;frame<40;frame++){
  LpState s;s.active=LP_TRACK_COUNT?0:-1;s.present=true;s.playing=true;s.positionMs=12000+frame*100;
  lpRender(pixels,s,0);char path[1024];snprintf(path,sizeof path,"%s/spectrum-%03u.ppm",argv[1],frame);if(!saveFrame(path))return 3;
 }
 return 0;
}
