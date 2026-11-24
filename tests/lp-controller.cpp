#include <cassert>
#include <stdint.h>
#include <cstring>
#include <cstdio>
struct LpTrack {uint8_t uid[10],uidLength;uint16_t trackNumber;uint32_t durationMs;};
const LpTrack lpTracks[]={{{1,2,3,4},4,1,10000},{{4,1,2,3,4,5,6},7,2,20000}};
constexpr uint16_t LP_TRACK_COUNT=2;
#include "controller.h"
int main(){
 LpState p;uint8_t a[]={1,2,3,4},b[]={4,1,2,3,4,5,6},unknown[]={9,9,9,9};
 assert(p.toggle(0)==LpEvent::None&&!p.playing);
 assert(p.scan(a,4,100)==LpEvent::None&&p.active==0&&!p.playing);
 assert(p.toggle(100)==LpEvent::Start&&p.playing);
 assert(p.scan(a,4,250)==LpEvent::None&&p.elapsed(250)==150);
 assert(p.toggle(260)==LpEvent::Pause&&p.manualPause&&p.positionMs==160);
 p.scan(a,4,500);assert(!p.playing&&p.manualPause);
 assert(p.tick(801)==LpEvent::None&&p.present&&p.manualPause&&p.positionMs==160);
 assert(p.scan(a,4,850)==LpEvent::None&&!p.playing);
 assert(p.toggle(850)==LpEvent::Resume&&p.elapsed(850)==160);
 assert(p.scan(b,7,900)==LpEvent::Stop&&p.active==1&&p.positionMs==0&&!p.playing);
 assert(p.toggle(900)==LpEvent::Start);
 assert(p.scan(unknown,4,950)==LpEvent::Pause&&p.unknown&&!p.playing);
 assert(p.toggle(960)==LpEvent::None);
 assert(p.scan(b,7,1000)==LpEvent::None&&!p.playing);
 assert(p.toggle(1000)==LpEvent::Resume&&p.elapsed(1000)==50);
 p.scan(b,7,1200);assert(p.tick(1400)==LpEvent::None&&p.playing);
 assert(p.tick(1501)==LpEvent::None&&p.playing);
 p.volume=30;p.adjustVolume(1);assert(p.volume==30);p.volume=0;p.adjustVolume(-1);assert(p.volume==0);
 // Completion clears the selection, blocks play, and requires a fresh scan.
 p.scan(a,4,1600);assert(p.toggle(1600)==LpEvent::Start);
 for(uint32_t n=1700;n<11600;n+=100){p.scan(a,4,n);p.tick(n);}
 p.scan(a,4,11600);assert(p.tick(11600)==LpEvent::Stop);
 assert(p.active==-1&&!p.present&&!p.playing&&!p.manualPause);
 assert(p.positionMs==0&&p.anchorMs==0&&p.lastSeenMs==0&&p.volume==0);
 assert(p.toggle(11601)==LpEvent::None&&!p.playing);
 assert(p.scan(a,4,11700)==LpEvent::None&&p.active==0&&!p.playing);
 assert(p.scan(a,4,11701)==LpEvent::None&&!p.playing);
 assert(p.toggle(11702)==LpEvent::Start&&p.positionMs==0);
 // UID lengths and millis wrap are handled exactly.
 assert(p.find(b,4)==-1);
 LpState ready;ready.volume=22;ready.scan(a,4,100);
 assert(ready.tick(60099)==LpEvent::None&&ready.active==0);
 assert(ready.tick(60100)==LpEvent::Stop&&ready.active==-1&&!ready.present&&ready.volume==22);
 assert(ready.toggle(60101)==LpEvent::None);
 // A successful scan refreshes the full minute, including the same UID.
 ready.scan(a,4,70000);ready.scan(a,4,120000);
 assert(ready.tick(130000)==LpEvent::None&&ready.active==0);
 assert(ready.tick(180000)==LpEvent::Stop&&ready.active==-1);
 // A replacement selection gets a fresh timeout and remains unstarted.
 ready.scan(a,4,200000);ready.scan(b,7,250000);
 assert(ready.active==1&&!ready.started&&!ready.playing);
 assert(ready.tick(260000)==LpEvent::None&&ready.active==1);
 assert(ready.tick(310000)==LpEvent::Stop&&ready.active==-1);
 // Even a pause at position zero must not be mistaken for an unstarted LP.
 ready.scan(a,4,400000);ready.toggle(400000);ready.toggle(400000);
 assert(ready.tick(500000)==LpEvent::None&&ready.active==0&&ready.manualPause);
 assert(ready.toggle(500000)==LpEvent::Start&&ready.playing);
 ready.clearSelection();ready.scan(a,4,0xffffff80u);
 assert(ready.tick(uint32_t(0xffffff80u+59999u))==LpEvent::None);
 assert(ready.tick(uint32_t(0xffffff80u+60000u))==LpEvent::Stop&&ready.active==-1);
 puts("PASS: UID matching/switch, unknown tags, NFC gate, latched pause, resume, clip end, ready timeout, volume and clock wrap");
}
