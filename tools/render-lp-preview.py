#!/usr/bin/env python3
from pathlib import Path
import subprocess,tempfile,json
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parent.parent
subprocess.run(['node','tools/check-lp-player.mjs'],cwd=root,check=True)
folder=root/'projects/lp-player/preview';folder.mkdir(parents=True,exist_ok=True)
with tempfile.TemporaryDirectory(prefix='lp-render-') as temp:
 binary=Path(temp)/'renderer'
 subprocess.run(['c++','-std=c++11','-O2','-Ifirmware/lp_player','tools/lp-player/render-preview.cpp','-o',str(binary)],cwd=root,check=True)
 subprocess.run([str(binary),str(folder)],check=True)
package=json.loads((root/'projects/lp-player/validation.json').read_text())['packageId'][2:]
contact=Image.new('RGB',(1280,1000),'#111827');draw=ImageDraw.Draw(contact)
for i,label in enumerate(['PLAYING','PAUSED','WAITING FOR LP','UNREGISTERED UID']):
 image=Image.open(folder/f'frame-{i}.ppm');image.resize((960,720),Image.Resampling.NEAREST).save(folder/f'frame-{i}-{package}.png')
 x=(i%2)*640;y=(i//2)*500;draw.text((x+12,y+8),label,fill='white');contact.paste(image.resize((640,480),Image.Resampling.NEAREST),(x,y+20))
contact.save(folder/f'contact-sheet-{package}.png')
frames=[Image.open(folder/f'spectrum-{i:03d}.ppm').resize((640,480),Image.Resampling.NEAREST).convert('RGB') for i in range(40)]
palette=frames[0].quantize(colors=128)
animation=[frame.quantize(palette=palette,dither=Image.Dither.NONE) for frame in frames]
animation[0].save(folder/f'spectrum-{package}.gif',save_all=True,append_images=animation[1:],duration=100,loop=0,optimize=True)
print(folder/f'frame-0-{package}.png')
