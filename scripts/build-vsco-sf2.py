#!/usr/bin/env python3
"""Reproduce selected CC0 VSCO 2 CE banks from tag 1.1.0 SFZ/WAV sources.
Usage: build-vsco-sf2.py SOURCE_DIR OUTPUT_DIR
Only the opcodes present in these three source maps are supported. Recordings
remain finite, as in the original SFZ: no artificial looping or synthesized bow.
"""
from pathlib import Path
import sys,re,struct,subprocess,array,json,hashlib
src,out=map(Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
def chunk(tag,data):return tag+struct.pack('<I',len(data))+data+(b'\0' if len(data)%2 else b'')
def name(s):return s.encode('ascii')[:19].ljust(20,b'\0')
def gen(op,value):return struct.pack('<HH',op,value&65535)
def timecents(seconds):
 import math
 return round(1200*math.log2(seconds))
manifest=[]
for source,title in [('ViolinEnsSusVib','ViolinSection'),('CelloEnsSusVib','CelloSection'),('SViolinVib','SoloViolin')]:
 text=(src/(source+'.sfz')).read_text();base=re.search(r'default_path=(.*)',text)[1].strip().replace('\\','/')
 regions=[];pcm=bytearray();headers=[];cache={};igen=bytearray();ibag=[]
 for block in text.split('<region>')[1:]:
  data=dict(re.findall(r'^(\w+)=(.*)',block,re.M));data={k:v.strip() for k,v in data.items()}
  allowed={'sample','lokey','hikey','lovel','hivel','pitch_keycenter','volume','tune'}
  assert not set(data)-allowed, set(data)-allowed
  path=src/'vsco'/base/data['sample'];key=(str(path),data.get('volume','0'))
  if key not in cache:
   raw=subprocess.check_output(['ffmpeg','-v','error','-i',str(path),'-f','s16le','-ac','2','-ar','44100','-'])
   samples=array.array('h');samples.frombytes(raw)
   if sys.byteorder!='little':samples.byteswap()
   # One fixed gain factor for the entire adaptation, then original per-region SFZ volume.
   scale=.2*10**(float(data.get('volume',0))/20);channels=[]
   for c in range(2):
    values=array.array('h',(round(x*scale) for x in samples[c::2]));assert max(map(abs,values),default=0)<32768
    if sys.byteorder!='little':values.byteswap()
    offset=len(pcm)//2;count=len(values);pcm.extend(values.tobytes());pcm.extend(b'\0'*92)
    idx=len(headers);headers.append(struct.pack('<20sIIIIIBbHH',name(f'{idx}-{c}'),offset,offset+count,offset,offset+count,44100,int(data['pitch_keycenter']),0,0,1));channels.append(idx)
   cache[key]=channels
   manifest.append({'source':base+data['sample'],'sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
  for c,sample_id in enumerate(cache[key]):
   ibag.append(struct.pack('<HH',len(igen)//4,0))
   values=[(43,int(data['lokey'])|(int(data['hikey'])<<8)),(44,int(data['lovel'])|(int(data['hivel'])<<8)),
           (17,-500 if c==0 else 500),(34,timecents(.001)),(38,timecents(.8)),(36,timecents(100)),(37,0),
           (58,int(data['pitch_keycenter'])),(52,round(float(data.get('tune',0)))),(54,0),(53,sample_id)]
   for op,val in values:igen.extend(gen(op,val))
 ibag.append(struct.pack('<HH',len(igen)//4,0));igen.extend(gen(0,0))
 headers.append(struct.pack('<20sIIIIIBbHH',name('EOS'),len(pcm)//2,len(pcm)//2,0,0,44100,0,0,0,1))
 pdta=b''
 tables=[(b'phdr',struct.pack('<20sHHHIII',name(title),0,0,0,0,0,0)+struct.pack('<20sHHHIII',name('EOP'),0,0,1,0,0,0)),
 (b'pbag',struct.pack('<HHHH',0,0,1,0)),(b'pmod',b'\0'*10),(b'pgen',gen(41,0)+gen(0,0)),
 (b'inst',struct.pack('<20sH',name(title),0)+struct.pack('<20sH',name('EOI'),len(ibag)-1)),
 (b'ibag',b''.join(ibag)),(b'imod',b'\0'*10),(b'igen',bytes(igen)),(b'shdr',b''.join(headers))]
 for tag,value in tables:pdta+=chunk(tag,value)
 info=chunk(b'ifil',struct.pack('<HH',2,1))+chunk(b'INAM',title.encode()+b'\0')+chunk(b'ICOP',b'CC0: Versilian Studios / Sam Gossner; adapted from VSCO 2 CE 1.1.0\0')
 body=b'sfbk'+chunk(b'LIST',b'INFO'+info)+chunk(b'LIST',b'sdta'+chunk(b'smpl',bytes(pcm)))+chunk(b'LIST',b'pdta'+pdta)
 target=out/(title+'.sf2');target.write_bytes(chunk(b'RIFF',body));print(target,len(body),flush=True)
(out/'VSCO-source-checksums.json').write_text(json.dumps(manifest,indent=2)+'\n')
