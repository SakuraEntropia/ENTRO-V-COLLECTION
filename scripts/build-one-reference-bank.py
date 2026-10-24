#!/usr/bin/env python3
"""Adapt a curated subset of the official Moog One 1.8.2 pack to this engine.
The official archive remains outside the repository. Unsupported routes/effects
are not copied or represented as exact; see docs/PRESET-REFERENCES.md.
Usage: python3 scripts/build-one-reference-bank.py /path/to/Moog_One_v1.8.2.zip
"""
import json, math, pathlib, sys, zipfile
SELECTED=[('CARPENTER BASS','BA - Cinema Pulse Ref','Bass'),('MINIBASS','BA - Small Ladder Ref','Bass'),
('BIG UNISON BASS X2','BA - Stacked Foundation Ref','Bass'),('SUPERSYNC','LD - Sync Crest Ref','Lead'),
('OB STRINGPAD','PD - Vintage Stringfield Ref','Pad'),("TOM & BOB'S BRASS",'BR - Dual Heritage Ref','Brass'),
('SUITCASE EP','KY - Suitcase Bloom Ref','Keys'),('A MOOD BEYOND','PD - Beyond Mood Ref','Pad'),
('MOONDAWN','PD - Lunar Dawn Ref','Pad'),('JUPITER WAVES','PD - Planet Waves Ref','Pad'),
("80'S BALLAD GUITAR",'KY - Ballad Harmonics Ref','Keys'),('GLASSWORKS','KY - Glass Assembly Ref','Keys')]

def parse(data):
    result={}
    for line in data.decode('utf-8-sig').splitlines():
        if ' ' not in line: continue
        key,value=line.split(' ',1)
        try:result[key]=json.loads(value.split(' #',1)[0])
        except (ValueError,TypeError):pass
    return result

def adapt(d):
    out={'one.master.volume':.57,'one.master.voices':1,'one.master.drift':.035,
         'one.master.reverbMix':.17,'one.master.reverbSize':.72,'one.master.reverbDamping':.38}
    for part in range(1,4):
        base=f'/p/part/t/{part}/'; p=base+'p/'; own=f'one.p{part}.'
        def get(k,default=0):return d.get(p+k,default)
        def put(k,v):
            if isinstance(v,(int,float)) and math.isfinite(v):out[own+k]=round(float(v),6)
        active=d.get(base+'en',False) # host MIDI replaces the hardware's local-port gating
        put('enabled',active);put('midiChannel',0);put('level',min(1,max(.1,get('g',.7))));put('pan',max(-1,min(1,2*get('pan',.5)-1)))
        mono=d.get(base+'a/mono',False);unison=d.get(base+'a/unison/en',False)
        put('voice.mode',2 if mono and unison else 1 if mono else 0)
        put('voice.unisonVoices',max(2,min(8,d.get(base+'a/unison/max-stack',2))));put('voice.unisonDetune',min(1,max(0,d.get(base+'a/unison/pitch-spread',.05))))
        for osc in range(1,4):
            src=f'o{osc}/'; dst=f'vco{osc}.'
            total=12*get(src+'oct',0)+get(src+'po',0)+get('po',0)
            octave=max(-1,min(2,round(total/12))); tune=max(-24,min(24,total-octave*12))
            put(dst+'octave',octave+1);put(dst+'tune',tune)
            # Our saw -> triangle angle occupies 0..0.5; source tr is
            # triangle -> saw. This is a timbral adaptation, not circuit equivalence.
            put(dst+'waveAngle',.5*(1-max(0,min(1,get(src+'tr',1)))))
            put(dst+'shape',max(0,min(1,get(f'mix/o{osc}/plsbal',0))))
            put(dst+'pw',max(.05,min(.95,get(src+'d',.5))))
            routed=get(f'mix/o{osc}/fa',True) or get(f'mix/o{osc}/fb',True)
            put(dst+'level',max(0,min(1,get(f'mix/o{osc}/g',.6))) if routed else 0)
            put(dst+'beatHz',max(-20,min(20,get(src+'fo',0))))
            if osc>1:put(dst+'sync',get(src+'sync1',False))
        put('vco3.fm',max(0,min(1,get('fm/amt',0))))
        put('mix.noise',get('mix/n/g',0) if get('mix/n/fa',False) or get('mix/n/fb',False) else 0)
        put('mix.ring',get('mix/rm/g',0) if get('mix/rm/fa',False) or get('mix/rm/fb',False) else 0)
        put('mix.ext',0);put('svf.level',1)
        put('filters.routing',0 if get('fafb',True) else 1);put('filters.mix',max(0,min(1,get('fbal',.5))))
        for src,dst in [('fa','svf'),('fb','ladder')]:
            note=get(src+'/p',80)
            put(dst+'.cutoff',max(20,min(20000,440*2**((note-69)/12))));put(dst+'.resonance',max(0,min(.9,get(src+'/r',.2))))
            put(dst+'.envAmt',max(-1,min(1,get(src+'/eg_p',.2))));put(dst+'.keyTrack',max(0,min(1,get(src+'/p_fp',.4))))
        mode=0
        if get('fa/1/hp',False):mode=1
        elif get('fa/1/bp',False):mode=2
        put('svf.mode',mode);put('svf.slope',1);put('ladder.slope',max(0,min(3,get('fb/tap',4)-1)));put('ladder.drive',.14)
        if get('fb/hp',False):
            # This engine's ladder is LP-only. Use the actual HP-capable SVF
            # path instead of turning an original 20-Hz HP into a muted LP.
            put('filters.routing',1);put('filters.mix',0);put('svf.mode',1)
            put('svf.cutoff',out[own+'ladder.cutoff'])
            put('svf.envAmt',out[own+'ladder.envAmt'])

        for hardware,ours in [(1,1),(1,2),(2,3)]:
            for a,b,default in [('delay','delay',0),('a','attack',.005),('h','hold',0),('d','decay',.5),('s','sustain',.7),('r','release',.3),('velmod','velocity',.2)]:
                val=get(f'e{hardware}/{a}',default)
                lo,hi=(.001,20) if a in ('a','d','r') else (0,1) if a in ('s','velmod') else (0,2)
                put(f'env{ours}.{b}',max(lo,min(hi,val)))
        for i in range(1,5):
            put(f'lfo{i}.rate',max(.01,min(100,get(f'l{i}/f',1))));put(f'lfo{i}.wave',{'sin':0,'tri':1,'saw':2,'ramp':3,'pulse':4,'sh':5}.get(get(f'l{i}/w','tri'),1))
            put(f'lfo{i}.sync',get(f'l{i}/sync',False));put(f'lfo{i}.fade',min(5,max(0,get(f'l{i}/tfin',0))))
        put('lfo1.pitchAmt',min(.08,max(0,get('m/l1_p',0))))
        put('lfo3.cutoffAmt',min(.25,max(0,get('fb/lfo_p',0))))
        # Gentle independent ambience; original Eventide algorithms unsupported.
        put('fx.chorus.mix',.1 if not mono else .02);put('fx.chorus.rate',.24);put('fx.chorus.depth',.22)
        put('fx.delay.time',.32);put('fx.delay.mix',.08);put('fx.delay.feedback',.25);put('fx.reverb.send',.18)
    return out

def main():
    archive=zipfile.ZipFile(sys.argv[1]);paths={pathlib.PurePosixPath(n).stem:n for n in archive.namelist() if n.endswith('.m1p')}
    rows=[]
    for original,name,category in SELECTED:
        if original not in paths:raise SystemExit('Missing reference: '+original)
        d=parse(archive.read(paths[original]));values=adapt(d)
        if original == 'BIG UNISON BASS X2':
            # Original matrix performance routes are not reproduced. Open the
            # otherwise near-muted LP base for this independent bass study.
            values['one.p1.svf.cutoff']=180.0
            values['one.p1.ladder.cutoff']=180.0
        rows.append((name,category,values,original))
    target=pathlib.Path(__file__).resolve().parents[1]/'src/plugin/OneReferencePresets.cpp'
    lines=['// Generated by scripts/build-one-reference-bank.py. Numerical adaptations, not original preset files.',
           '#include "plugin/Presets.h"','namespace triad {','std::vector<FactoryPreset> oneReferencePresets() {','    return {']
    for name,category,values,original in rows:
        lines+=['        // Reference: '+original,'        { '+json.dumps(name)+', '+json.dumps(category)+', {']
        lines += ['            { '+json.dumps(k)+', '+format(v,'.7g')+('f' if '.' in format(v,'.7g') or 'e' in format(v,'.7g') else '.0f')+' },' for k,v in values.items()]
        lines+=['        } },']
    lines+=['    };','}','}'];target.write_text('\n'.join(lines)+'\n')
    print('Created',len(rows),'reference adaptations:',target)
if __name__=='__main__':main()
