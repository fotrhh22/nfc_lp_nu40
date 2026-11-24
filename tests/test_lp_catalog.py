import json,subprocess,sys,tempfile,unittest,wave
from pathlib import Path
from PIL import Image
SCRIPT=Path(__file__).resolve().parents[1]/'tools/lp-catalog.py'
class CatalogTests(unittest.TestCase):
    def test_multiple_registration_uniqueness_and_replace_preserves_other_song(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);catalog=root/'catalog.json';audio=root/'source.wav';cover=root/'source.png'
            with wave.open(str(audio),'wb') as f:
                f.setnchannels(1);f.setsampwidth(2);f.setframerate(44100);f.writeframes(b'\0'*(4410*2))
            Image.new('RGB',(8,8),'blue').save(cover)
            def add(ident,uid,*extra):
                return subprocess.run([sys.executable,str(SCRIPT),'--catalog',str(catalog),'add','--id',ident,'--uid',uid,'--audio',str(audio),'--cover',str(cover),'--title',ident,'--artist','Artist',*extra],capture_output=True,text=True)
            self.assertEqual(add('one','11:22:33:44').returncode,0)
            self.assertEqual(add('two','04AABBCCDDEEFF').returncode,0)
            data=json.loads(catalog.read_text());self.assertEqual(len(data['tracks']),2)
            self.assertEqual([t['trackNumber'] for t in data['tracks']],[1,2])
            for track in data['tracks']:
                self.assertTrue((root/track['cover']).is_file());self.assertTrue((root/track['audio']).is_file())
            before=catalog.read_bytes()
            self.assertNotEqual(add('duplicate','11223344').returncode,0)
            self.assertNotEqual(add('one','11223344').returncode,0)
            self.assertNotEqual(add('zero','99887766','--number','0').returncode,0)
            self.assertEqual(catalog.read_bytes(),before)
            self.assertEqual(add('one','AABBCCDD','--replace').returncode,0)
            data=json.loads(catalog.read_text());self.assertEqual(len(data['tracks']),2)
            self.assertEqual(data['tracks'][0]['uid'],'AABBCCDD');self.assertEqual(data['tracks'][1]['id'],'two')
            self.assertEqual(data['tracks'][0]['trackNumber'],1)
            self.assertGreaterEqual(len(list((root/'catalog-backups').glob('*.json'))),2)
if __name__=='__main__':unittest.main()
