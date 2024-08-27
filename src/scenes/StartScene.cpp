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
  textGeneratorAccent.set_center_alignment();
  textGenerator.set_z_order(-1);
  textGenerator.set_bg_priority(0);
  textGeneratorAccent.set_z_order(-1);
  textGeneratorAccent.set_bg_priority(0);

  printCredits();
  hideCredits();
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
    for (int i = 0; i < 5; i++)
      addExplosion();
  }

  // Background
  pixelBlink->update();
  updateVideo();

  // Explosions
  iterate(explosions, [](Explosion* it) { return it->update(); });

  // Credits
  if (bn::keypad::l_pressed()) {
    credits = !credits;
    if (credits)
      showCredits();
    else
      hideCredits();
  }

  // Connect/disconnect detection
  if (!isConnected && linkUniversal->isConnected()) {
    isConnected = true;
    onConnected();
  } else if (isConnected && !linkUniversal->isConnected()) {
    isConnected = false;
    onDisconnected();
  }

  // Link session
  if (isConnected) {
    // Send/receive/test
    unsigned otherPlayerId = !linkUniversal->currentPlayerId();

    if (!error) {
      // Send counters
      send();
      send();

      // Send commands
      if (bn::keypad::left_held()) {
        // Send <move left> command
        linkUniversal->send((1 << 15) | 1);
      } else if (bn::keypad::right_held()) {
        // Send <move right> command
        linkUniversal->send((1 << 15) | 2);
      }

      // Receive commands
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
    if (!credits) {
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
      textGeneratorAccent.generate({0, -70}, "L: Toggle credits", textSprites);
      textGenerator.generate({0, -30}, output1, textSprites);
      textGenerator.generate({0, -10}, output2, textSprites);
      textGenerator.generate({0, 10}, output3, textSprites);
    }
  }

  // Horse
  horse->setPosition({horse->getPosition().x(), HORSE_Y}, true);
  horse->update();
  horse->setFlipX(horse->getCenteredPosition().x() > 0);
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
  explosions.push_back(bn::unique_ptr{new Explosion(
      {random.get_fixed(-100, 100), random.get_fixed(-100, 100)})});
}

void StartScene::print(bn::string<128> text) {
  if (credits)
    return;
  textSprites.clear();
  textGenerator.generate({0, -30}, text, textSprites);
}

void StartScene::showCredits() {
  for (auto& it : textSprites)
    it.set_visible(false);
  for (auto& it : creditsSprites)
    it.set_visible(true);
}

void StartScene::hideCredits() {
  for (auto& it : textSprites)
    it.set_visible(true);
  for (auto& it : creditsSprites)
    it.set_visible(false);
}

void StartScene::printCredits() {
  creditsSprites.clear();
  textGenerator.generate({0, -70 - 5}, "github.com/afska/", creditsSprites);
  textGeneratorAccent.generate({0, -60 - 5}, "gba-link-universal-test",
                               creditsSprites);

  textGenerator.generate({0, -40 - 5}, "Game engine:", creditsSprites);
  textGeneratorAccent.generate({0, -30 - 5}, "Butano (@GValiente)",
                               creditsSprites);

  textGenerator.generate({0, -10 - 5}, "Background music:", creditsSprites);
  textGeneratorAccent.generate({0, 0 - 5}, "Lazer Idols (@Synthenia)",
                               creditsSprites);

  textGenerator.generate({0, 20 - 5}, "Background video:", creditsSprites);
  textGeneratorAccent.generate({0, 30 - 5}, "@RoyaltyFreeTube", creditsSprites);

  textGenerator.generate({0, 50 - 5}, "Horse:", creditsSprites);
  textGeneratorAccent.generate({0, 60 - 5}, "@Lu", creditsSprites);

  textGenerator.generate({0, 70}, "Read the #licenses folder!", creditsSprites);
}
