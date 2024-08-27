# gba-link-universal-test

This is a test ROM (based on [BeatBeast](https://github.com/afska/beat-beast)) that tests how the [LinkUniversal](https://github.com/afska/gba-link-connection?tab=readme-ov-file#-LinkUniversal) library behaves in the context of a game, with an audio player (in Direct Sound DMA mode) playing music, a background video, text and sprites.

Made with [Butano](https://github.com/GValiente/butano). Check out the `#licenses` folder for details!

- Uses *butano* `17.7.0` and *gba-link-connection* `7.0.0`.
- Compiled with devkitPro, using GCC `14.1.0` with `-Ofast` as the optimization level.