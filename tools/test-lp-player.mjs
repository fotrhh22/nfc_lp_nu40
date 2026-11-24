import './check-lp-player.mjs';
import {execFileSync} from 'node:child_process';
import {mkdtempSync,rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import path from 'node:path';
const temp=mkdtempSync(path.join(tmpdir(),'lp-tests-'));
try{
 execFileSync('.venv/bin/python',['tests/test_lp_catalog.py'],{stdio:'inherit'});
 execFileSync('.venv/bin/python',['tests/test_spectrum.py'],{stdio:'inherit'});
 for(const [name,source,includes] of [
  ['controller','tests/lp-controller.cpp',['tools/lp-player']],
  ['firmware','tests/lp-firmware.cpp',['tests/host-stubs','firmware/lp_player']]
 ]){
  const output=path.join(temp,name);execFileSync('c++',['-std=c++11','-O2',...includes.map(p=>'-I'+p),source,'-o',output],{stdio:'inherit'});execFileSync(output,[],{stdio:'inherit'});
 }
}finally{rmSync(temp,{recursive:true,force:true});}
