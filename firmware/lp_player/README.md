# LP album player firmware

Delivery: lp_player.ino and lp_assets.h, kept together. Generated from this
project's tools/lp-player/ sources and private projects/lp-player/catalog.json.
There is no lyric animation or runtime dependency on another project.

Edit the catalog/source, then run lp:generate, lp:test, lp:preview and lp:compile
from the project root. Do not directly edit the generated pair. Stale-output
checks include catalog, audio, cover, source, font and spectrum inputs.

The album screen, NFC presence gate, manual pause reset, P1.05 play/pause,
P1.06/07 volume, touch calibration and DFPlayer Serial1 audio are included.
Cover/title are embedded in Flash. Actual MP3s must be copied manually to the
DFPlayer microSD as /mp3/0001.mp3, etc. The spectrum uses precomputed source FFT
frames; its progress clock is estimated from command timing.

See ../../README.md and ../../docs/WIRING.md for setup, commands and wiring.
Building does not upload. Before any requested upload verify the connected
board and preserve its recovery path. Existing-board recovery is under
projects/recovery/ relative to the project root.
