# Next steps

1. Add the remaining 4–5 songs with actual sticker UIDs, audio and album covers.
2. Run lp:sd, lp:test, lp:preview and lp:compile; review Flash/RAM for the complete library.
3. With module power off, manually copy sd-card/mp3 onto the physical microSD.
4. Only when upload is requested: verify connected NU40 target/serial, recovery
   artifact, then upload this project's LP firmware pair.
5. Test every sticker, correct song/cover, manual play/pause, 60-second ready timeout, RFID gaps during pause/play,
   volume limits, touch calibration, audible playback and spectrum/audio timing.
6. Verify final enclosure RF reading and battery power design before sealing.

Normal playback requires no computer after music and firmware are installed.
New sticker/song registration in this version is a pre-assembly PC workflow.

Planned hardware/software: add physical buttons and a motor/driver after choosing
power requirements and pins; link motor output to playback state; handle actual
DFPlayer completion/errors; add RFID read/gap diagnostics. Do not infer physical
LP presence from a cached UID during playback or pause.
