"""Offline stereo-energy STFT spectrum: 20 log-spaced bands, 100ms frames."""
import subprocess
import numpy as np

BANDS=20
FRAME_MS=100
LOW_HZ=50.0
HIGH_HZ=16000.0

def analyze_spectrum(path):
    rate,nfft=44100,2048
    raw=subprocess.run(['ffmpeg','-v','error','-i',str(path),'-map','0:a:0','-f','f32le','-acodec','pcm_f32le','-ar',str(rate),'-ac','2','pipe:1'],stdout=subprocess.PIPE,check=True).stdout
    samples=np.frombuffer(raw,dtype='<f4').reshape(-1,2)
    if not len(samples):raise ValueError('No decoded audio: '+str(path))
    hop=rate*FRAME_MS//1000
    count=(len(samples)+hop-1)//hop
    edges=np.geomspace(LOW_HZ,HIGH_HZ,BANDS+1)
    freqs=np.fft.rfftfreq(nfft,1/rate)
    masks=[np.flatnonzero((freqs>=edges[i])&(freqs<edges[i+1])) for i in range(BANDS)]
    if any(not len(mask) for mask in masks):raise ValueError('FFT too short for requested band layout')
    window=np.hanning(nfft)
    # Parseval normalization gives windowed band mean-square energy; combining
    # channel power preserves opposite-phase stereo instead of cancelling it.
    norm=2.0/(nfft*np.sum(window*window))
    padded=np.pad(samples,((nfft//2,nfft),(0,0)))
    energy=np.empty((count,BANDS),dtype=np.float64)
    for first in range(0,count,128):
        ids=np.arange(first,min(count,first+128))*hop
        frames=padded[ids[:,None]+np.arange(nfft)[None,:],:]
        fft=np.fft.rfft(frames*window[None,:,None],axis=1)
        power=np.mean(np.abs(fft)**2,axis=2)*norm
        for band,mask in enumerate(masks):energy[first:first+len(ids),band]=power[:,mask].sum(axis=1)
    amplitude=np.sqrt(np.maximum(energy,0))
    reference=max(float(np.percentile(amplitude,99.5)),0.01)
    # Linear amplitude does not visually inflate weak components as a wide dB
    # scale does. One reference is shared by every band in this song.
    level=np.clip(amplitude/reference,0,1)
    # Fast attack / slower release, applied only to the analyzed signal.
    smooth=np.empty_like(level);previous=np.zeros(BANDS)
    for i,current in enumerate(level):
        weight=np.where(current>previous,0.85,0.35)
        previous=previous+weight*(current-previous);smooth[i]=previous
    packed=np.rint(smooth*255).astype(np.uint8)
    return {'method':'Hann STFT stereo band energy, logarithmic frequency bands, linear RMS amplitude display, attack/release smoothing',
            'sampleRate':rate,'fftSize':nfft,'frameMs':FRAME_MS,'bands':BANDS,'frameCount':count,
            'sourceDurationMs':round(len(samples)*1000/rate),
            'bandEdgesHz':[round(float(v),3) for v in edges],
            'displayScale':'linear RMS amplitude; song-wide 99.5th percentile reference',
            'displayReferenceRms':round(reference,8),
            'data':packed.tobytes()}
