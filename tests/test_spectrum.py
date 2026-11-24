import importlib.util,math,struct,tempfile,unittest,wave
from pathlib import Path
import numpy as np
spec=importlib.util.spec_from_file_location('lp_spectrum',Path(__file__).resolve().parents[1]/'tools/lp-player/spectrum.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
class SpectrumTests(unittest.TestCase):
    def analyze(self,segments):
        with tempfile.TemporaryDirectory() as folder:
            path=Path(folder)/'tones.wav';rate=44100;data=bytearray()
            for frequency,amplitude in segments:
                for i in range(rate):
                    value=round(amplitude*math.sin(2*math.pi*frequency*i/rate)*32767)
                    data.extend(struct.pack('<hh',value,-value))
            with wave.open(str(path),'wb') as f:
                f.setnchannels(2);f.setsampwidth(2);f.setframerate(rate);f.writeframes(data)
            result=module.analyze_spectrum(path)
            return result,np.frombuffer(result['data'],dtype=np.uint8).reshape(-1,result['bands'])
    def test_bass_to_treble_and_silence_with_opposite_phase_stereo(self):
        result,levels=self.analyze([(220,.5),(6000,.5),(0,0)])
        edges=result['bandEdgesHz'];band=lambda hz:int(np.searchsorted(edges,hz)-1)
        self.assertEqual(result['sourceDurationMs'],3000);self.assertEqual(result['frameCount'],30)
        self.assertEqual(int(np.argmax(levels[5])),band(220))
        self.assertEqual(int(np.argmax(levels[15])),band(6000))
        self.assertGreater(int(levels[5].max()),200)
        self.assertLess(int(levels[29].max()),10)
        self.assertFalse(np.array_equal(levels[5],levels[15]))
    def test_silence_has_no_random_bars(self):
        result,levels=self.analyze([(0,0)])
        self.assertEqual(levels.shape,(10,20));self.assertEqual(int(levels.max()),0)
    def test_stronger_tone_raises_same_frequency_band(self):
        result,levels=self.analyze([(1000,.05),(1000,.5)])
        band=int(np.searchsorted(result['bandEdgesHz'],1000)-1)
        self.assertGreater(int(levels[15,band])-int(levels[5,band]),65)
if __name__=='__main__':unittest.main()
