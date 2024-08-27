# gba-link-universal-test

This is a test ROM (based on [BeatBeast](https://github.com/afska/beat-beast)) that checks how the [LinkUniversal](https://github.com/afska/gba-link-connection?tab=readme-ov-file#-LinkUniversal) library behaves in the context of a 2-player game, with an audio player (in Direct Sound DMA mode) playing music, a background video, text and sprites.

The example sets the interval at around ~4 transfers per frame and sends 3 consecutive numbers on each frame. Each node validates that the numbers are consecutive, ensuring there are no repeated or missed packets. No audio pops should be audible on hardware.

- A pre-compiled ROM is available in the *Releases* section.
- Made with [Butano](https://github.com/GValiente/butano). Check out the `#licenses` folder for details!
- Uses *butano* `17.7.0` and *gba-link-connection* `7.0.0`.
- Compiled with devkitPro, using GCC `14.1.0` with `-Ofast` as the optimization level.