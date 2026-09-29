# Vendor audio extractor

`extract_vendor_audio.py` exports the barter/store dialogue referenced by the retail `brbs_*.csv` BotTalk tables. It resolves GameCube DSP-ADPCM samples through the relevant SFX banks and `snd_init.rdg`, writes standalone WAVs, and creates a CSV manifest plus extraction report.

```powershell
python tools/extract_vendor_audio.py
```

By default it reads `gamedata/mst` and writes to `build/export/slim_shady_audio`. Use `--data` and `--output` to select other paths. The extractor does not launch the game or change game data.
