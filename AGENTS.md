# NFC LP Table

- Read README.md, docs/STATUS.md and docs/NEXT_STEPS.md first.
- This is an independent UID album player; do not depend on the original lyrics-studio, F1 or feelgood folders.
- Browser/computer UI automation is not authorized. Use terminal tools and local generated images.
- Editable sources: tools/lp-player/ and projects/lp-player/catalog.json. Delivery: firmware/lp_player/lp_player.ino + lp_assets.h.
- Regenerate after source/catalog/audio/cover changes; run freshness checks and appropriate tests/compile.
- Preview shares C++ raster rendering but does not prove physical TFT/audio/touch behavior or alignment.
- Do not claim hardware builds/uploads/tests succeeded without actual evidence.
- Upload replaces firmware: confirm target and recovery first. Never automatically upload or write/format a physical SD during generation/build.
- Keep private audio/covers/catalogs, generated outputs and sessions out of source control by default; never silently upload personal media.
