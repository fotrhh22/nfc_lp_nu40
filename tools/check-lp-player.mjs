import {readFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import path from 'node:path';
const root=new URL('../',import.meta.url),manifest=JSON.parse(await readFile(new URL('projects/lp-player/validation.json',root)));
for(const [name,hash] of Object.entries({...manifest.sourceHashes,...manifest.outputHashes})){
 const p=path.isAbsolute(name)?name:new URL(name,root);
 if(createHash('sha256').update(await readFile(p)).digest('hex')!==hash)throw Error(`STALE LP player: ${name}. Run npm run lp:generate.`);
}
console.log(`PASS: LP player ${manifest.packageId}; ${manifest.tracks.length} registered UID(s); sources/assets match.`);
