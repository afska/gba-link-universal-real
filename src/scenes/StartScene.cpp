#include "StartScene.h"

#include "../utils/Math.h"
#include "../utils/gba-link-connection/LinkUniversal.hpp"

#include "bn_keypad.h"

#include "bn_music_items.h"

StartScene::StartScene(const GBFS_FILE* _fs) : VideoScene(_fs) {}

void StartScene::init() {
  linkUniversal->activate();  // << enable LinkUniversal

  VideoScene::init();

  printCredits();
  hideCredits();
  onDisconnected();
}

void StartScene::destroy() {
  linkUniversal->deactivate();  // << disable LinkUniversal
}

void StartScene::update() {
  linkUniversal->sync();  // << update LinkUniversal

  VideoScene::update();

  // Credits
  if (bn::keypad::select_pressed()) {
    credits = !credits;
    if (credits)
      showCredits();
    else
      hideCredits();
  }

  // Multiboot
  if (bn::keypad::l_pressed()) {
    setNextScreen(Screen::MULTIBOOT_CABLE);
  } else if (bn::keypad::r_pressed())
    setNextScreen(Screen::MULTIBOOT_WIRELESS);

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
          "Waiting... [" + bn::to_string<128>((int)linkUniversal->getState()) +
          "]";
      output1 += "<" + bn::to_string<128>((int)linkUniversal->getMode()) + ">";
      if (linkUniversal->getMode() == LinkUniversal::Mode::LINK_WIRELESS)
        output1 += " (" +
                   bn::to_string<128>((int)linkUniversal->getWirelessState()) +
                   ")";
      bn::string<128> output2 =
          "_wait: " + bn::to_string<128>(linkUniversal->_getWaitCount());
      bn::string<128> output3 =
          "_subW: " + bn::to_string<128>(linkUniversal->_getSubWaitCount());
      textSprites.clear();
      textGenerator.generate({0, -30}, output1, textSprites);
      textGenerator.generate({0, -10}, output2, textSprites);
      textGenerator.generate({0, 10}, output3, textSprites);
    }
  }
}

void StartScene::send() {
  unsigned oldCounter = counter;
  counter = (counter + 1) & ~(1 << 15);
  // (^^ we'll use bit 15 for special commands for this demo)

  if (!linkUniversal->send(counter))
    counter = oldCounter;
}

void StartScene::onConnected() {
  print("Whoa! Connected!");
  pixelBlink->blink();
  bn::music_items::cyberrid.play(1);
}

void StartScene::onError(unsigned expected, unsigned actual) {
  print("Expected " + bn::to_string<128>(expected) + " but got " +
        bn::to_string<128>(actual));
  textGenerator.generate(
      {0, 0},
      "Queue overflow: " +
          bn::to_string<32>(linkUniversal->didQueueOverflow(false)),
      textSprites);
  pixelBlink->blink();
}

void StartScene::onDisconnected() {
  print("Waiting...");
  error = false;
  counter = 0;
  received = 0;
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

  uiTextSprites.clear();
}

void StartScene::hideCredits() {
  for (auto& it : textSprites)
    it.set_visible(true);
  for (auto& it : creditsSprites)
    it.set_visible(false);

  textGeneratorAccent.generate({0, -70}, "SELECT: Toggle credits",
                               uiTextSprites);
  textGeneratorAccent.generate({0, -70 + 10}, "L/R: Multiboot (Cable/RFU)",
                               uiTextSprites);
}

void StartScene::printCredits() {
  creditsSprites.clear();
  textGenerator.generate({0, -70 - 4}, "github.com/afska/", creditsSprites);
  textGeneratorAccent.generate({0, -60 - 4}, "gba-link-universal-test",
                               creditsSprites);

  textGenerator.generate({0, -40 - 4}, "Game engine:", creditsSprites);
  textGeneratorAccent.generate({0, -30 - 4}, "Butano (@GValiente)",
                               creditsSprites);

  textGenerator.generate({0, -10 - 4}, "Background music:", creditsSprites);
  textGeneratorAccent.generate({0, 0 - 4}, "cyberrid (@Jester)",
                               creditsSprites);

  textGenerator.generate({0, 20 - 4}, "Background video:", creditsSprites);
  textGeneratorAccent.generate({0, 30 - 4}, "@RoyaltyFreeTube", creditsSprites);

  textGenerator.generate({0, 50 - 4}, "Horse:", creditsSprites);
  textGeneratorAccent.generate({0, 60 - 4}, "@Lu", creditsSprites);

  textGenerator.generate({0, 70}, "Read the #licenses folder!", creditsSprites);
}
