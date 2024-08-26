#include "StartScene.h"

#include "../assets/StartVideo.h"
#include "../assets/fonts/common_fixed_8x16_sprite_font.h"
#include "../assets/fonts/common_fixed_8x16_sprite_font_accent.h"
#include "../player/player.h"
#include "../utils/Math.h"
#include "../utils/gba-link-connection/LinkUniversal.hpp"
#include "bn_memory.h"

#include "bn_blending.h"
#include "bn_keypad.h"

#define HORSE_X 40
#define HORSE_Y 90
#define BPM 115
#define BEAT_PREDICTION_WINDOW 100

StartScene::StartScene(const GBFS_FILE* _fs)
    : Scene(_fs),
      horse(bn::unique_ptr{new Horse({0, 0})}),
      textGenerator(common_fixed_8x16_sprite_font),
      textGeneratorAccent(common_fixed_8x16_sprite_font_accent),
      pixelBlink(bn::unique_ptr{new PixelBlink(0.5)}) {
  horse->showGun = false;
  horse->setPosition({HORSE_X, HORSE_Y}, true);
  horse->update();
  updateVideo();
  textGenerator.set_center_alignment();
}

void StartScene::init() {
  player_playPCM("dj.pcm");
  player_setLoop(true);
  onDisconnected();
}

void StartScene::update() {
  // Horse
  horse->setPosition({HORSE_X, HORSE_Y}, true);
  horse->update();

  // Rhythm
  const int PER_MINUTE = 71583;  // (1/60000) * 0xffffffff
  int audioLag = 0;              // (0 on real hardware)
  int msecs = PlaybackState.msecs - audioLag + BEAT_PREDICTION_WINDOW;
  int beat = Math::fastDiv(msecs * BPM, PER_MINUTE);
  bool isNewBeat = beat != lastBeat;
  lastBeat = beat;
  if (isNewBeat)
    extraSpeed = 10;
  if (isNewBeat)
    horse->jump();

  // Background
  pixelBlink->update();
  updateVideo();

  // Link
  if (!error) {
    if (!isConnected && linkUniversal->isConnected()) {
      isConnected = true;
      onConnected();
    } else if (isConnected && !linkUniversal->isConnected()) {
      isConnected = false;
      onDisconnected();
    }
  }

  if (isConnected) {
    unsigned otherPlayerId = !linkUniversal->currentPlayerId();

    if (!error) {
      linkUniversal->send(++counter);
      linkUniversal->send(++counter);
      linkUniversal->send(++counter);

      while (linkUniversal->canRead(otherPlayerId)) {
        unsigned receivedNumber = linkUniversal->read(otherPlayerId);
        if (receivedNumber > received + 1) {
          error = true;
          onError(received + 1, receivedNumber);
          break;
        } else {
          received = receivedNumber;
          print(bn::to_string<128>(received));
        }
      }
    }
  }
}

void StartScene::onConnected() {
  print("Whoa! Connected!");
  pixelBlink->blink();
  player_seek(0);
}

void StartScene::onError(unsigned expected, unsigned actual) {
  print("Expected " + bn::to_string<128>(expected) + " but got " +
        bn::to_string<128>(actual));
  pixelBlink->blink();
}

void StartScene::onDisconnected() {
  print("Waiting...");
  error = false;
  counter = 0;
  received = 0;
}

void StartScene::updateVideo() {
  background.reset();
  background = StartVideo::getFrame(videoFrame.floor_integer())
                   .create_bg((256 - Math::SCREEN_WIDTH) / 2,
                              (256 - Math::SCREEN_HEIGHT) / 2);
  background.get()->set_mosaic_enabled(true);
  extraSpeed = (bn::max(extraSpeed - 1, bn::fixed(0)));
  videoFrame += (1 + extraSpeed / 2) / 2;
  if (videoFrame >= 150)
    videoFrame = 0;

  auto alpha = 0.7 - bn::fixed(extraSpeed) / 20;
  if (alpha > 1)
    alpha = 1;
  if (alpha < 0)
    alpha = 0;
  bn::blending::set_transparency_alpha(alpha);
}

void StartScene::print(bn::string<128> text) {
  textSprites.clear();
  textGenerator.generate({0, -30}, text, textSprites);
}
