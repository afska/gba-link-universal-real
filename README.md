# LinkUniversal_real

This is a test ROM (based on [BeatBeast](https://github.com/afska/beat-beast)) that checks how the [LinkUniversal](https://github.com/afska/gba-link-connection?tab=readme-ov-file#-LinkUniversal) library behaves in the context of a 2-player game, with an audio player (in Direct Sound DMA mode) playing music, a background video, text and sprites.

The example sets the interval at around ~4 transfers per frame and sends 2 consecutive numbers on each frame. Each node validates that the numbers are consecutive, ensuring there are no repeated or missed packets. Also, each player can send extra packets to move the remote horse to the left or right. No audio pops should be audible on hardware.

As an extra option, support for `LinkCableMultiboot::Async` and `LinkWirelessMultiboot::Async` has been added to send a multiboot ROM asynchronously, while keeping the audio and animations running.

- A pre-compiled ROM is available in the [Releases](https://github.com/afska/gba-link-universal-real/releases) section.
- Check out the `USERFLAGS` line in the [Makefile](Makefile#L51) to see the selected `LinkWireless` **build configuration**.
- Check out the [main.cpp](src/main.cpp#L32) file to see the **runtime configuration**.
- Uses *butano* `18.7.1` and *gba-link-connection* `8.0.0`.
  * The only difference with the mainstream butano is that this repo has a [GBFS integration](https://github.com/afska/gba-link-universal-real/commit/3f00a407b007f3efbb0c016f2932ca1a26d8ab14).
- Compiled with [devkitARM](https://devkitpro.org), using GCC `14.1.0` with `-Ofast` as the optimization level.
- Check out the `#licenses` folder for details.
