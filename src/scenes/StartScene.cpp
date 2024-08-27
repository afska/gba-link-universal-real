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
#define BPM 85
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
  player_playPCM("lazer.pcm");
  player_setLoop(true);
  onDisconnected();
}

void StartScene::update() {
  // Rhythm
  const int PER_MINUTE = 71583;  // (1/60000) * 0xffffffff
  int audioLag = 0;              // (0 on real hardware)
  int msecs = PlaybackState.msecs - audioLag + BEAT_PREDICTION_WINDOW;
  int beat = Math::fastDiv(msecs * BPM, PER_MINUTE);
  bool isNewBeat = beat != lastBeat;
  lastBeat = beat;
  if (isNewBeat)
    extraSpeed = 10;
  if (isNewBeat) {
    horse->jump();
    addExplosion();
    addExplosion();
    addExplosion();
    addExplosion();
    addExplosion();
  }

  // Background
  pixelBlink->update();
  updateVideo();

  // Explosions
  iterate(explosions, [](Explosion* it) { return it->update(); });

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
    // Send/receive/test
    unsigned otherPlayerId = !linkUniversal->currentPlayerId();

    if (!error) {
      // Send counter
      send();
      send();
      send();

      // Send commands
      if (bn::keypad::left_held()) {
        // send <move left> command
        linkUniversal->send((1 << 15) | 1);
      } else if (bn::keypad::right_held()) {
        // send <move right> command
        linkUniversal->send((1 << 15) | 2);
      }

      while (linkUniversal->canRead(otherPlayerId)) {
        unsigned receivedNumber = linkUniversal->read(otherPlayerId);

        if (receivedNumber & (1 << 15)) {
          // received command (move horse)
          unsigned direction = receivedNumber & ~(1 << 15);
          horse->setPosition(
              {horse->getPosition().x() + (direction == 1 ? -5 : 5), HORSE_Y},
              true);
          if (horse->getPosition().x() < 0)
            horse->setPosition({0, HORSE_Y}, true);
          if (horse->getPosition().x() > 240)
            horse->setPosition({240, HORSE_Y}, true);
        } else {
          // received counter
          unsigned expectedNumber = received + 1;
          if (expectedNumber != (1 << 15) && receivedNumber != expectedNumber) {
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
  } else {
    // Debug output
    bn::string<128> output1 =
        "Waiting... [" + bn::to_string<128>(linkUniversal->getState()) + "]";
    output1 += "<" + bn::to_string<128>(linkUniversal->getMode()) + ">";
    if (linkUniversal->getMode() == LinkUniversal::Mode::LINK_WIRELESS)
      output1 +=
          " (" + bn::to_string<128>(linkUniversal->getWirelessState()) + ")";
    bn::string<128> output2 =
        "_wait: " + bn::to_string<128>(linkUniversal->_getWaitCount());
    bn::string<128> output3 =
        "_subW: " + bn::to_string<128>(linkUniversal->_getSubWaitCount());
    textSprites.clear();
    textGenerator.generate({0, -30}, output1, textSprites);
    textGenerator.generate({0, -10}, output2, textSprites);
    textGenerator.generate({0, 10}, output3, textSprites);
  }

  // Horse
  horse->setPosition({horse->getPosition().x(), HORSE_Y}, true);
  horse->update();
}

void StartScene::send() {
  counter = (counter + 1) & ~(1 << 15);
  // (^^ we'll use bit 15 for special commands for this demo)

  linkUniversal->send(counter);
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

void StartScene::addExplosion() {
  explosions.push_back(bn::unique_ptr{
      new Explosion({random.get_fixed(-50, 50), random.get_fixed(-50, 50)})});
}

void StartScene::print(bn::string<128> text) {
  textSprites.clear();
  textGenerator.generate({0, -30}, text, textSprites);
}
