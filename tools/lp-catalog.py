#!/usr/bin/env python3
"""Import local audio/cover files and register multiple independent UID entries."""
import argparse,hashlib,json,os,re,shutil,subprocess,tempfile
from pathlib import Path
from datetime import datetime
from PIL import Image
ROOT=Path(__file__).resolve().parent.parent

def normalized_uid(value):
    value=re.sub(r'[\s:-]','',value).upper()
    if not re.fullmatch('[0-9A-F]+',value) or len(value) not in (8,14,20):
        raise ValueError('UID must contain exactly 4, 7 or 10 bytes of hexadecimal')
    return value

def imported(source,folder,ident):
    source=Path(source).expanduser().resolve();data=source.read_bytes();digest=hashlib.sha256(data).hexdigest()
    folder.mkdir(parents=True,exist_ok=True);target=folder/f'{ident}-{digest[:12]}{source.suffix.lower()}'
    if target.exists():
        if hashlib.sha256(target.read_bytes()).hexdigest()!=digest:raise ValueError('Imported filename collision')
    else:shutil.copyfile(source,target)
    return target

def register(args):
    path=Path(args.catalog).resolve();path.parent.mkdir(parents=True,exist_ok=True)
    data=json.loads(path.read_text()) if path.exists() else {'version':1,'tracks':[]}
    entries=data['tracks'];old=next((v for v in entries if v['id']==args.id),None)
    if not re.fullmatch(r'[a-z0-9][a-z0-9_-]*',args.id):raise ValueError('--id: use lowercase letters, digits, hyphen or underscore')
    if old and not args.replace:raise ValueError('ID already exists; use --replace to update it')
    if args.replace and not old:raise ValueError('--replace requires an existing ID')
    uid=normalized_uid(args.uid);others=[v for v in entries if v is not old]
    if any(normalized_uid(v['uid'])==uid for v in others):raise ValueError('UID already registered to another song')
    used={v['trackNumber'] for v in others}
    number=args.number if args.number is not None else (old['trackNumber'] if old else next((n for n in range(1,3001) if n not in used),None))
    if number is None or not 1<=number<=3000 or number in used:raise ValueError('MP3 number must be unused and between 1 and 3000')
    audio=Path(args.audio).expanduser().resolve();cover=Path(args.cover).expanduser().resolve() if args.cover else None
    probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_entries','format=duration:stream=codec_type','-of','json',str(audio)],text=True))
    if not any(s.get('codec_type')=='audio' for s in probe.get('streams',[])):raise ValueError('No audio stream in input file')
    duration=round(float(probe['format']['duration'])*1000)
    if cover:
        with Image.open(cover) as image:image.verify()
    # Validate all inputs/unique keys before importing or changing the registry.
    target_audio=imported(audio,path.parent/'audio',args.id)
    target_cover=imported(cover,path.parent/'covers',args.id) if cover else None
    entry=dict(old or {})
    entry.update({'id':args.id,'uid':uid,'trackNumber':number,'title':args.title,'artist':args.artist,'durationMs':duration,'audio':str(target_audio.relative_to(path.parent))})
    entry['cover']=str(target_cover.relative_to(path.parent)) if target_cover else (old.get('cover') if old else None)
    if target_cover:entry['coverKind']='custom'
    if old:entries[entries.index(old)]=entry
    else:entries.append(entry)
    if path.exists():
        backups=path.parent/'catalog-backups';backups.mkdir(exist_ok=True)
        shutil.copyfile(path,backups/f'catalog-{datetime.now().strftime("%Y%m%d-%H%M%S-%f")}.json')
    with tempfile.NamedTemporaryFile('w',dir=path.parent,delete=False,suffix='.json',encoding='utf8') as stream:
        json.dump(data,stream,indent=2,ensure_ascii=False);stream.write('\n');temporary=stream.name
    os.replace(temporary,path)
    print(f'Registered {args.id}: UID {uid} -> /mp3/{number:04d}.mp3 ({args.title} / {args.artist})')
    print('Catalog changed: regenerate/check/compile/upload firmware; run lp:sd for local SD staging. No board or physical SD was written.')

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--catalog',default=str(ROOT/'projects/lp-player/catalog.json'))
    commands=parser.add_subparsers(dest='command',required=True)
    add=commands.add_parser('add')
    for name in ['id','uid','audio','title','artist']:add.add_argument('--'+name,required=True)
    add.add_argument('--cover');add.add_argument('--number',type=int);add.add_argument('--replace',action='store_true')
    commands.add_parser('list')
    args=parser.parse_args()
    try:
        if args.command=='add':register(args)
        else:
            path=Path(args.catalog);entries=json.loads(path.read_text())['tracks'] if path.exists() else []
            for entry in entries:print(f'{entry["id"]}: {entry["uid"]} -> {entry["trackNumber"]:04d} | {entry["title"]} / {entry["artist"]} | {entry.get("cover")}')
            print(f'{len(entries)} registered song(s)')
    except (ValueError,KeyError,OSError,subprocess.CalledProcessError) as error:parser.exit(1,str(error)+'\n')
if __name__=='__main__':main()
