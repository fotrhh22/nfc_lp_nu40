struct LpCanvas {
 uint16_t* pixels;
 static uint16_t rgb(uint8_t r,uint8_t g,uint8_t b){return uint16_t((r>>3)<<11|(g>>2)<<5|(b>>3));}
 void pixel(int x,int y,uint16_t c){if(x>=0&&x<320&&y>=0&&y<240)pixels[y*320+x]=c;}
 void rect(int x,int y,int w,int h,uint16_t c){for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)pixel(i,j,c);}
 void line(int x0,int y0,int x1,int y1,uint16_t c){int dx=abs(x1-x0),sx=x0<x1?1:-1,dy=-abs(y1-y0),sy=y0<y1?1:-1,err=dx+dy;for(;;){pixel(x0,y0,c);if(x0==x1&&y0==y1)break;int e=2*err;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}}}
 void circle(int cx,int cy,int r,uint16_t c){for(int y=-r;y<=r;y++)for(int x=-r;x<=r;x++)if(x*x+y*y<=r*r)pixel(cx+x,cy+y,c);}
 void text(int x,int y,const char* s,uint16_t c){for(;*s;s++,x+=6){unsigned v=static_cast<unsigned char>(*s);if(v<32||v>126)v='?';for(int j=0;j<11;j++)for(int i=0;i<6;i++)if(lpFont[v-32][j]&(1<<i))pixel(x+i,y+j,c);}}
 void mask(int x,int y,const uint8_t* data,int w,int h,uint16_t c){for(int j=0;j<h;j++)for(int i=0;i<w;i++)if(data[(j*w+i)/8]&(0x80>>((j*w+i)%8)))pixel(x+i,y+j,c);}
};
static inline void lpTime(uint32_t ms,char* output){snprintf(output,12,"%lu:%02lu",(unsigned long)(ms/60000),(unsigned long)((ms/1000)%60));}
static inline void lpRender(uint16_t* pixels,const LpState& state,uint32_t now){
 const uint16_t bg=0x0863,white=0xef7e,muted=0x7c51,cyan=0x4e9f,pink=0xec95,dim=0x2128;
 LpCanvas c{pixels};c.rect(0,0,320,240,bg);
 c.text(14,8,"LP / PLAYER",muted);
 if(state.active<0&&!state.unknown){
  c.text(136,98,"PLACE LP",white);
  c.text(115,122,"WAITING FOR NFC",muted);
  return;
 }
 const char* status=state.unknown?"UNREGISTERED":(state.present?"NFC PRESENT":"PLACE LP");
 c.text(306-int(strlen(status))*6,8,status,state.present?cyan:pink);
 const LpTrack* track=state.active>=0&&!state.unknown?&lpTracks[state.active]:nullptr;
 if(track){
  for(int y=0;y<112;y++)for(int x=0;x<112;x++)c.pixel(14+x,30+y,track->cover[y*112+x]);
  c.mask(142,31,track->titleMask,164,48,white);c.mask(142,82,track->artistMask,164,18,muted);
 }else{
  c.rect(14,30,112,112,dim);c.circle(70,86,42,0x10a5);c.circle(70,86,23,0x318b);c.circle(70,86,5,pink);
  c.text(142,46,state.unknown?"UNKNOWN STICKER":"YOUR NEXT RECORD",white);
  c.text(142,68,state.unknown?"UID NOT REGISTERED":"PLACE A REGISTERED",muted);
  c.text(142,82,state.unknown?"TRY ANOTHER LP":"LP ON THE READER",muted);
 }
 uint32_t elapsed=track?state.elapsed(now):0,total=track?track->durationMs:1;if(elapsed>total)elapsed=total;
 // Horizontal axis is frequency (50Hz -> 16kHz), not song progress.
 c.rect(142,142,164,1,dim);
 if(track&&track->spectrumFrames){
  uint32_t frame=elapsed/LP_SPECTRUM_FRAME_MS;if(frame>=track->spectrumFrames)frame=track->spectrumFrames-1;
  const uint8_t* values=track->spectrum+frame*LP_SPECTRUM_BANDS;
  for(uint8_t band=0;band<LP_SPECTRUM_BANDS;band++){
   // Keep the linear source-amplitude scale; only the visual shape changes.
   const int h=(unsigned(values[band])*34+127)/255;
   if(h){c.rect(142+band*8,142-h,6,h,white);c.rect(142+band*8,142-h,6,1,cyan);}
  }
 }
 c.text(142,145,"LOW",muted);c.text(282,145,"HIGH",muted);
 c.rect(16,158,288,3,dim);int progress=int(uint64_t(elapsed)*288/total);c.rect(16,158,progress,3,cyan);c.circle(16+progress,159,3,cyan);
 char current[12],duration[12];lpTime(elapsed,current);lpTime(track?track->durationMs:0,duration);
 c.text(16,169,current,muted);c.text(304-int(strlen(duration))*6,169,duration,muted);
 c.circle(160,210,21,cyan);c.circle(160,210,19,bg);
 if(state.playing){c.rect(153,202,4,16,white);c.rect(164,202,4,16,white);}
 else for(int dx=0;dx<=13;dx++)c.line(155+dx,201+dx*9/13,155+dx,219-dx*9/13,white);
 c.rect(16,204,5,8,muted);c.line(21,204,27,199,muted);c.line(27,199,27,217,muted);c.line(27,217,21,212,muted);
 c.rect(37,204,70,5,dim);c.rect(37,204,state.volume*70/30,5,pink);
 char volume[12];snprintf(volume,sizeof volume,"VOL %02u",state.volume);c.text(37,214,volume,muted);
 c.rect(214,195,36,30,dim);c.rect(228,208,10,3,white);
 c.rect(270,195,36,30,dim);c.rect(282,208,12,3,white);c.rect(287,203,3,13,white);
}
// Only these controls are interactive: play/pause and volume -/+.
static inline int lpHitControl(int x,int y){
 if(x>=137&&x<=183&&y>=187&&y<=233)return 1;
 if(x>=214&&x<250&&y>=195&&y<225)return -1;
 if(x>=270&&x<306&&y>=195&&y<225)return 2;
 return 0;
}
