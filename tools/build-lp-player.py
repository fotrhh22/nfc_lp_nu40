#!/usr/bin/env python3
"""Generate the two-file LP player, raster assets and source-aware manifest locally."""
import argparse,hashlib,json,math,re,subprocess,sys
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont,ImageOps
ROOT=Path(__file__).resolve().parent.parent
FONT=Path('/System/Library/Fonts/AppleSDGothicNeo.ttc')
MONO=Path('/System/Library/Fonts/Menlo.ttc')
sys.path.insert(0,str(ROOT/'tools/lp-player'))
from spectrum import analyze_spectrum
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def packed(image):
 bits=[1 if v>100 else 0 for v in image.convert('L').tobytes()];out=bytearray((len(bits)+7)//8)
 for i,v in enumerate(bits):
  if v:out[i//8]|=0x80>>(i%8)
 return out

def label(text,size,height):
 im=Image.new('L',(164,height));draw=ImageDraw.Draw(im);font=ImageFont.truetype(str(FONT),size)
 # Wrap to pixel width and scale font down when a long title needs more room.
 for font_size in range(size,9,-1):
  font=ImageFont.truetype(str(FONT),font_size);lines=['']
  for ch in text:
   if draw.textlength(lines[-1]+ch,font=font)>162 and lines[-1]:lines.append(ch)
   else:lines[-1]+=ch
  if len(lines)*(font_size+2)<=height:break
 if len(lines)*(font_size+2)>height:raise ValueError('Title/artist too long for display: '+text)
 for i,line in enumerate(lines):draw.text((0,i*(font_size+2)),line,font=font,fill=255)
 return packed(im)

def placeholder(index):
 im=Image.new('RGB',(112,112));p=im.load()
 for y in range(112):
  for x in range(112):
   t=(x+y)/222;p[x,y]=(int(82+118*t),int(194-104*t),int(243-23*t))
 d=ImageDraw.Draw(im)
 for radius in [43,39,35,31,27,23]:d.ellipse((56-radius,56-radius,56+radius,56+radius),outline=(184,220,255),width=1)
 d.ellipse((43,43,69,69),fill=(90,135,230));d.ellipse((53,53,59,59),fill=(225,243,255))
 return im

def arr(name,values,ctype='uint8_t'):
 values=list(values);digits=4 if ctype=='uint16_t' else 2
 lines=[f'const {ctype} {name}[] = {{']
 for i in range(0,len(values),20):lines.append(','.join(f'0x{x:0{digits}x}' for x in values[i:i+20])+',')
 return '\n'.join(lines+['};'])

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--catalog',default='projects/lp-player/catalog.json');parser.add_argument('--prepare-sd',action='store_true');args=parser.parse_args()
 catalog=(ROOT/args.catalog).resolve()
 if not catalog.exists():catalog=ROOT/'config/lp-catalog.example.json'
 data=json.loads(catalog.read_text());tracks=data.get('tracks',[]);seen=set();numbers=set();ids=set();asset_bytes=0
 inputs=[Path(__file__),catalog,FONT,MONO]+[ROOT/'tools/lp-player'/name for name in ['player.ino.template','controller.h','ui.h','audio.inc','touch.inc','spectrum.py']]
 cpp=['#pragma once','#include <stdint.h>','#include <stddef.h>','#include <string.h>','#include <stdlib.h>','#include <stdio.h>']
 rows=[];manifest_tracks=[]
 for i,track in enumerate(tracks):
  if track['id'] in ids:raise ValueError('Duplicate song ID '+track['id'])
  ids.add(track['id'])
  uid=re.sub(r'[\s:-]','',track['uid']).upper()
  if not re.fullmatch(r'[0-9A-F]+',uid) or len(uid) not in (8,14,20):raise ValueError('UID must contain 4, 7 or 10 bytes')
  if uid in seen:raise ValueError('Duplicate UID '+uid)
  seen.add(uid);number=track['trackNumber'];duration=track['durationMs']
  if type(number)!=int or not 1<=number<=3000 or number in numbers:raise ValueError('Each track needs a distinct MP3 number 1..3000')
  if type(duration)!=int or not 0<duration<86400000:raise ValueError('Invalid durationMs')
  numbers.add(number)
  cover=(catalog.parent/track['cover']).resolve() if track.get('cover') else None
  if cover:inputs.append(cover)
  image=ImageOps.fit(Image.open(cover).convert('RGB'),(112,112)) if cover else placeholder(i)
  raw=image.tobytes();pixels=[((raw[j]>>3)<<11)|((raw[j+1]>>2)<<5)|(raw[j+2]>>3) for j in range(0,len(raw),3)]
  cpp.extend([arr(f'lpCover{i}',pixels,'uint16_t'),arr(f'lpTitle{i}',label(track['title'],22,48)),arr(f'lpArtist{i}',label(track['artist'],13,18))])
  uid_bytes=list(bytes.fromhex(uid));uid_values=','.join(str(v) for v in uid_bytes+[0]*(10-len(uid_bytes)))
  cpp_id=json.dumps(track['id'],ensure_ascii=True)
  audio=(catalog.parent/track['audio']).resolve();inputs.append(audio)
  spectrum=analyze_spectrum(audio)
  if abs(spectrum['sourceDurationMs']-duration)>1000:raise ValueError('durationMs differs from actual audio by more than 1s: '+track['id'])
  cpp.append(arr(f'lpSpectrum{i}',spectrum['data']))
  asset_bytes+=len(pixels)*2+len(label(track['title'],22,48))+len(label(track['artist'],13,18))+len(spectrum['data'])
  if asset_bytes>600*1024:raise ValueError('Album covers/spectra exceed the 600KiB asset budget; reduce catalog size before compiling')
  rows.append(f'{{{{{uid_values}}},{len(uid_bytes)},{number},{duration}u,lpCover{i},lpTitle{i},lpArtist{i},lpSpectrum{i},{spectrum["frameCount"]}u,{cpp_id}}}')
  manifest_tracks.append({'id':track['id'],'trackNumber':number,'uid':uid,'durationMs':duration,'placeholderCover':cover is None or track.get('coverKind')=='placeholder','spectrum':{k:v for k,v in spectrum.items() if k!='data'}})
  if args.prepare_sd:
   target=ROOT/f'projects/lp-player/sd-card/mp3/{number:04d}.mp3';target.parent.mkdir(parents=True,exist_ok=True)
   subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-i',str(audio),'-map','0:a:0','-c:a','libmp3lame','-b:a','192k','-ar','44100','-ac','2','-map_metadata','-1',str(target)],check=True)
 font=ImageFont.truetype(str(MONO),9);glyphs=[]
 for code in range(32,127):
  im=Image.new('L',(6,11));ImageDraw.Draw(im).text((0,-1),chr(code),font=font,fill=255)
  glyphs.append([sum((1<<x) for x in range(6) if im.getpixel((x,y))>100) for y in range(11)])
 cpp.append('const uint8_t lpFont[95][11]={'+','.join('{'+','.join(map(str,row))+'}' for row in glyphs)+'};')
 cpp.append('constexpr uint8_t LP_SPECTRUM_BANDS=20;')
 cpp.append('constexpr uint16_t LP_SPECTRUM_FRAME_MS=100;')
 cpp.append('struct LpTrack {uint8_t uid[10],uidLength;uint16_t trackNumber;uint32_t durationMs;const uint16_t* cover;const uint8_t* titleMask;const uint8_t* artistMask;const uint8_t* spectrum;uint32_t spectrumFrames;const char* songId;};')
 cpp.append('const LpTrack lpTracks[] = {'+(','.join(rows) if rows else '{}')+'};')
 cpp.append(f'constexpr uint16_t LP_TRACK_COUNT={len(rows)};')
 cpp.extend((ROOT/'tools/lp-player'/name).read_text() for name in ['controller.h','ui.h'])
 hashes={str(p.relative_to(ROOT)) if p.is_relative_to(ROOT) else str(p):digest(p) for p in inputs}
 package=hashlib.sha256(json.dumps(hashes,sort_keys=True).encode()).hexdigest()[:8]
 cpp.append(f'constexpr uint32_t LP_PACKAGE_ID=0x{package}u;')
 out=ROOT/'firmware/lp_player';out.mkdir(parents=True,exist_ok=True)
 (out/'lp_assets.h').write_text('\n'.join(cpp)+'\n')
 template=(ROOT/'tools/lp-player/player.ino.template').read_text()
 template=template.replace('@AUDIO_IMPLEMENTATION@',(ROOT/'tools/lp-player/audio.inc').read_text()).replace('@TOUCH_IMPLEMENTATION@',(ROOT/'tools/lp-player/touch.inc').read_text()).replace('@PACKAGE_ID@','0x'+package)
 (out/'lp_player.ino').write_text(template)
 manifest={'packageId':'0x'+package,'tracks':manifest_tracks,'sourceHashes':hashes,'outputHashes':{str(p.relative_to(ROOT)):digest(p) for p in [out/'lp_assets.h',out/'lp_player.ino']},'sharedRasterRenderer':True,'uploaded':False}
 private=ROOT/'projects/lp-player';private.mkdir(parents=True,exist_ok=True);(private/'validation.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n')
 print(json.dumps({'packageId':'0x'+package,'tracks':len(rows),'output':str(out),'sdFilesPrepared':args.prepare_sd}))
if __name__=='__main__':main()
