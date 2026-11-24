# Status — 2026-11-24

Independent NU40DK UID album player. Source tools, tests, hardware configuration
and example catalogs are versioned. Personal audio, covers, UID catalogs,
generated firmware/assets, previews, build outputs, recovery files and local
export reports are excluded.

Implemented: registered UID selects the cover/song without autoplay; a button
starts playback. Repeated reads keep the current position. A selection that has
never started expires after 60 seconds without another successful UID read.
Playback and manual pause survive missed RFID reads. Another registered UID
selects a new song and waits for a button. Song end clears selection and position,
blocks play until a fresh scan, and renders the text-only default screen.

Local freshness checks and registry/signal/controller/actual-sketch host tests
passed. The last local NU40DK compile passed with Flash 173172 bytes, static RAM
167540 bytes and 70028 bytes remaining. These results used the private local
library; generated outputs and media are not distributed through Git.

No board upload, physical SD write, TFT/audio/touch alignment or motor test was
performed. Physical buttons and motor have not been added. Motor control,
DFPlayer completion/error-driven state changes and RFID diagnostic logging remain
future work. Current completion uses the catalog duration, not an actual audio
completion event. Paused LP removal cannot be detected without another sensor.
