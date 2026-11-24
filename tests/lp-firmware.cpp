#include <cassert>
#include <algorithm>
#include <cstdio>
#include "Arduino.h"
#include "SPI.h"
uint64_t mockUs=0;bool mockPins[64];MockSerial Serial;MockSPI SPI;MockAudioSerial Serial1;
#define LP_HOST_TEST 1
#include "lp_player.ino"
static unsigned transfers=0;
void hostPresent(uint16_t* frame,uint16_t w,uint16_t h){assert(frame&&w==320&&h==240);++transfers;mockUs+=45000;}
static void tick(unsigned us){mockUs+=us;loop();}
static void flush(){for(int i=0;i<6;i++)tick(50000);}
static bool sent(uint8_t command,size_t start=0){for(size_t i=start;i<Serial1.packets.size();i++)if(Serial1.packets[i][3]==command)return true;return false;}
int main(){
 std::fill(mockPins,mockPins+64,HIGH);setup();assert(!player.playing&&transfers>0&&!sent(0x12));
 assert(LP_TRACK_COUNT>0);rfid.uid.size=lpTracks[0].uidLength;memcpy(rfid.uid.uidByte,lpTracks[0].uid,rfid.uid.size);rfid.hostCard=true;tick(110000);flush();assert(!player.playing&&!sent(0x12));
 togglePlayback();flush();assert(player.playing&&sent(0x12));
 mockPins[37]=LOW;loop();tick(40000);assert(player.manualPause&&!player.playing);mockPins[37]=HIGH;loop();tick(40000);flush();assert(sent(0x0e));
 const auto saved=player.positionMs;rfid.hostCard=false;tick(400000);assert(player.present&&player.manualPause&&player.positionMs==saved);
 const auto before=Serial1.packets.size();tick(61000000);assert(!player.playing&&player.manualPause);togglePlayback();flush();assert(player.playing&&sent(0x0d,before));rfid.hostCard=true;
 // Exact rendered hitboxes and touch mapping, with no seek/skip controls.
 assert(lpHitControl(160,210)==1&&lpHitControl(232,210)==-1&&lpHitControl(288,210)==2);
 assert(lpHitControl(180,159)==0&&lpHitControl(50,70)==0);
 touchReady=true;touchDown({2000,3650,true});assert(!player.playing&&player.manualPause);
 touchDown({2000,3650,true});assert(player.playing&&!player.manualPause);
 // Replace a queued play with pause if a card is removed before UART dispatch.
 audioCount=0;audioLoadedTrack=0;queueTransport(LpEvent::Start);assert(audioCount==1&&audioQueue[0].command==0x12);
 queueTransport(LpEvent::Pause);assert(audioCount==1&&audioQueue[0].command==0x0e);
 queueTransport(LpEvent::Resume);assert(audioCount==1&&audioQueue[0].command==0x12);
 queueAudio(0x06,17);queueAudio(0x06,18);assert(audioCount==2&&audioQueue[1].value==18);
 for(const auto& p:Serial1.packets){assert(p.size()==10&&p[0]==0x7e&&p[9]==0xef);uint16_t sum=0;for(int i=1;i<=6;i++)sum+=p[i];assert(uint16_t(0-sum)==uint16_t((p[7]<<8)|p[8]));}
 // Frequency bars point upward and freeze at the current MP3 time while paused.
 LpState graph;graph.active=0;graph.present=true;graph.positionMs=48500;
 lpRender(rgbFrame,graph,1000);std::vector<uint16_t> first(rgbFrame,rgbFrame+320*240);
 lpRender(rgbFrame,graph,9000);assert(std::equal(first.begin(),first.end(),rgbFrame));
 const auto& track=lpTracks[0];assert(track.spectrumFrames>1);
 bool nonzero=false;for(int y=108;y<142;y++)for(int x=142;x<306;x++)if(rgbFrame[y*320+x]!=0x0863)nonzero=true;assert(nonzero);
 // Columns are frequency bands; moving to a new sample changes heights, not a
 // left-to-right progress fill. The separate progress bar remains untouched.
 bool changed=false;
 for(uint32_t frame=0;frame<track.spectrumFrames;frame+=10){
  graph.positionMs=frame*LP_SPECTRUM_FRAME_MS;lpRender(rgbFrame,graph,9000);
  for(int y=108;y<142;y++)for(int x=142;x<306;x++)if(rgbFrame[y*320+x]!=first[y*320+x])changed=true;
  if(changed)break;
 }
 assert(changed);graph.positionMs=48500;
 graph.volume=30;lpRender(rgbFrame,graph,9000);
 for(int y=108;y<=142;y++)for(int x=142;x<306;x++)assert(rgbFrame[y*320+x]==first[y*320+x]);
 // End-of-song renders exactly the text-only initial screen, preserving volume.
 LpState initial;initial.volume=player.volume;
 lpRender(rgbFrame,initial,0);std::vector<uint16_t> waiting(rgbFrame,rgbFrame+320*240);
 player.active=0;player.present=true;player.playing=true;player.manualPause=false;
 player.positionMs=lpTracks[0].durationMs;player.anchorMs=millis();player.lastSeenMs=millis();
 audioCount=0;applyEvent(player.tick(millis()));
 assert(player.active==-1&&!player.present&&!player.playing&&player.positionMs==0);
 assert(audioCount==1&&audioQueue[0].command==0x16);
 lpRender(rgbFrame,player,millis());assert(std::equal(waiting.begin(),waiting.end(),rgbFrame));
 togglePlayback();assert(!player.playing&&audioCount==1);
 applyEvent(player.scan(lpTracks[0].uid,lpTracks[0].uidLength,millis()));
 assert(player.active==0&&!player.playing&&audioCount==1);
 // An unstarted LP expires to the same default frame after one minute.
 rfid.hostCard=false;const auto expires=player.lastSeenMs+60000;
 assert(player.tick(expires-1)==LpEvent::None&&player.active==0);
 audioCount=0;applyEvent(player.tick(expires));
 assert(player.active==-1&&!player.present&&uiDirty);
 lpRender(rgbFrame,player,expires);assert(std::equal(waiting.begin(),waiting.end(),rgbFrame));
 togglePlayback();assert(!player.playing);
 puts("PASS: LP firmware UID gate, buttons/touch, audio commands/queue cancellation, full-frame transfers");
}
