enum class LpEvent:uint8_t {None,Start,Resume,Pause,Stop};
struct LpState {
 int16_t active=-1;
 bool present=false,unknown=false,manualPause=false,playing=false,ended=false,started=false;
 uint8_t volume=15;
 uint32_t positionMs=0,anchorMs=0,lastSeenMs=0;
 uint32_t elapsed(uint32_t now)const{return positionMs+(playing?uint32_t(now-anchorMs):0);}
 void clearSelection(){active=-1;present=unknown=manualPause=playing=ended=started=false;positionMs=anchorMs=lastSeenMs=0;}
 LpEvent pause(uint32_t now){if(!playing)return LpEvent::None;positionMs=elapsed(now);playing=false;return LpEvent::Pause;}
 int find(const uint8_t* uid,uint8_t length)const{
  for(uint16_t i=0;i<LP_TRACK_COUNT;i++)if(lpTracks[i].uidLength==length&&!memcmp(lpTracks[i].uid,uid,length))return i;
  return -1;
 }
 LpEvent scan(const uint8_t* uid,uint8_t length,uint32_t now){
  int selected=find(uid,length);lastSeenMs=now;
  if(selected<0){LpEvent event=pause(now);present=false;unknown=true;manualPause=false;return event;}
  unknown=false;
  if(selected!=active){LpEvent event=playing?LpEvent::Stop:LpEvent::None;active=selected;present=true;manualPause=false;ended=started=false;positionMs=0;anchorMs=now;playing=false;return event;}
  present=true;
  return LpEvent::None;
 }
 LpEvent lost(uint32_t now){LpEvent event=pause(now);present=unknown=manualPause=false;return event;}
 LpEvent tick(uint32_t now){
  // Only an unstarted selection expires. Rotation may hide the tag during
  // playback or pause, so a missed read must not revoke an existing session.
  if(active>=0&&!started&&!unknown&&uint32_t(now-lastSeenMs)>=60000){clearSelection();return LpEvent::Stop;}
  if(unknown&&uint32_t(now-lastSeenMs)>=300)return lost(now);
  if(playing&&active>=0&&elapsed(now)>=lpTracks[active].durationMs){clearSelection();return LpEvent::Stop;}
  return LpEvent::None;
 }
 LpEvent toggle(uint32_t now){
  if(!present||active<0||unknown)return LpEvent::None;
  if(ended){ended=false;manualPause=false;positionMs=0;anchorMs=now;playing=true;return LpEvent::Start;}
  if(playing){manualPause=true;return pause(now);}
  manualPause=false;
  started=true;
  anchorMs=now;playing=true;return positionMs?LpEvent::Resume:LpEvent::Start;
 }
 void adjustVolume(int step){int value=int(volume)+step;volume=value<0?0:(value>30?30:value);}
};
