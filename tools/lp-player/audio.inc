struct AudioPacket {uint8_t command;uint16_t value;};
static AudioPacket audioQueue[8];static uint8_t audioCount=0,audioRx[10],audioRxSize=0;
static uint16_t audioLoadedTrack=0;static uint32_t lastAudioSend=0;
static bool isTransport(uint8_t c){return c==0x12||c==0x0d||c==0x0e||c==0x16;}
static void queueAudio(uint8_t command,uint16_t value){
 if(command==0x06)for(uint8_t i=0;i<audioCount;i++)if(audioQueue[i].command==command){audioQueue[i].value=value;return;}
 if(audioCount>=8){Serial.println("DFPlayer command queue full");return;}
 audioQueue[audioCount++]={command,value};
}
static void queueTransport(LpEvent event){
 // Replace pending transport intent so a removed card cannot leave a queued play.
 uint8_t keep=0;for(uint8_t i=0;i<audioCount;i++)if(!isTransport(audioQueue[i].command))audioQueue[keep++]=audioQueue[i];audioCount=keep;
 uint16_t track=player.active<0?0:lpTracks[player.active].trackNumber;
 if(event==LpEvent::Start)queueAudio(0x12,track);
 else if(event==LpEvent::Resume)queueAudio(audioLoadedTrack==track?0x0d:0x12,audioLoadedTrack==track?0:track);
 else if(event==LpEvent::Pause)queueAudio(0x0e,0);
 else if(event==LpEvent::Stop)queueAudio(0x16,0);
}
static void sendAudio(uint8_t command,uint16_t value){
 uint8_t p[10]={0x7e,0xff,6,command,1,uint8_t(value>>8),uint8_t(value),0,0,0xef};uint16_t sum=0;
 for(int i=1;i<=6;i++)sum+=p[i];sum=uint16_t(0-sum);p[7]=sum>>8;p[8]=sum;Serial1.write(p,10);
 if(command==0x12)audioLoadedTrack=value;if(command==0x16)audioLoadedTrack=0;
}
static void pumpAudio(uint32_t now){
 if(!audioCount||uint32_t(now-lastAudioSend)<40)return;lastAudioSend=now;
 AudioPacket p=audioQueue[0];for(uint8_t i=1;i<audioCount;i++)audioQueue[i-1]=audioQueue[i];--audioCount;sendAudio(p.command,p.value);
}
static void pollAudio(){
 while(Serial1.available()){
  uint8_t b=Serial1.read();if(!audioRxSize&&b!=0x7e)continue;audioRx[audioRxSize++]=b;
  if((audioRxSize==2&&b!=255)||(audioRxSize==3&&b!=6)){audioRxSize=0;continue;}
  if(audioRxSize!=10)continue;audioRxSize=0;uint16_t sum=0;for(int i=1;i<=6;i++)sum+=audioRx[i];
  if(audioRx[9]!=0xef||uint16_t(0-sum)!=uint16_t((audioRx[7]<<8)|audioRx[8]))continue;
  Serial.print("DFPlayer cmd=");Serial.print(audioRx[3]);Serial.print(" value=");Serial.println((audioRx[5]<<8)|audioRx[6]);
  if(audioRx[3]==0x40)Serial.println("DFPlayer returned an error; numeric code above");
 }
}
static void initAudio(){
 Serial1.begin(9600);delay(1500);sendAudio(0x09,2);delay(200);sendAudio(0x06,player.volume);delay(40);sendAudio(0x16,0);delay(40);queueAudio(0x48,0);
}
