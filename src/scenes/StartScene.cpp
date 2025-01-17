#include "StartScene.h"

#include "../utils/Math.h"
#include "../utils/gba-link-connection/LinkUniversal.hpp"
#include "../player/player.h"

#include "bn_keypad.h"

StartScene::StartScene(const GBFS_FILE* _fs)
    : VideoScene(_fs) {
}

void StartScene::init() {
  VideoScene::init();

  printCredits();
  hideCredits();
  onDisconnected();
}

void StartScene::update() {
  VideoScene::update();

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
