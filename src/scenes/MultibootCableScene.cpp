#include "MultibootCableScene.h"

#include "../player/player.h"
#include "../utils/Math.h"
#include "../utils/gba-link-connection/LinkWirelessMultiboot.hpp"

#include "bn_keypad.h"

#define FILE_NAME "LinkUniversal_fullmb.gba"

using u32 = Link::u32;
using u16 = Link::u16;
using u8 = Link::u8;

MultibootCableScene::MultibootCableScene(const GBFS_FILE* _fs)
    : VideoScene(_fs) {}

void MultibootCableScene::init() {
  VideoScene::init();
  printInstructions();
  horse->customScale = true;
}

void MultibootCableScene::update() {
  VideoScene::update();

  bool isSending = linkWirelessMultibootAsync->getState() !=
                   LinkWirelessMultiboot::Async::State::STOPPED;
  bool hasResult = linkWirelessMultibootAsync->getResult(false) !=
                   LinkWirelessMultiboot::Async::Result::NONE;

  if (hasResult) {
    uiTextSprites.clear();
    textGenerator.generate(
        {0, 0},
        "Result: " +
            bn::to_string<32>(linkWirelessMultibootAsync->getResult(false)),
        uiTextSprites);
    if (bn::keypad::a_pressed()) {
      linkWirelessMultibootAsync->getResult();
      printInstructions();
    }
    return;
  }

  if (isSending) {
    uiTextSprites.clear();
    auto percentage = linkWirelessMultibootAsync->getPercentage();
    horse->getMainSprite().set_scale(
        percentage > 0 ? 1 + bn::fixed(percentage) / 100 : 1);
    textGenerator.generate({0, -10},
                           "Sending... " + bn::to_string<32>(percentage) + "%",
                           uiTextSprites);
    textGenerator.generate(
        {0, 10},
        bn::to_string<32>(linkWirelessMultibootAsync->playerCount()) +
            " players",
        uiTextSprites);

    if (bn::keypad::start_pressed())
      linkWirelessMultibootAsync->markReady();

    if (bn::keypad::b_pressed()) {
      linkWirelessMultibootAsync->reset();
      printInstructions();
    }
  } else {
    if (bn::keypad::a_pressed())
      sendRom();

    if (bn::keypad::b_pressed())
      sendRom(true);

    if (bn::keypad::l_pressed())
      setNextScreen(Screen::MAIN);
  }
}

void MultibootCableScene::sendRom(bool normalMode) {
  unsigned long romSize;
  const u8* romToSend = (const u8*)gbfs_get_obj(fs, FILE_NAME, &romSize);

  linkWirelessMultibootAsync->sendRom(romToSend, romSize, "Multiboo",
                                      "Demodemo", 0x7FFF, 5,
                                      bn::keypad::start_held());
}

void MultibootCableScene::printInstructions() {
  uiTextSprites.clear();
  textGeneratorAccent.generate({0, -10}, "A = MULTI, B = SPI", uiTextSprites);
  textGenerator.generate({0, 10}, "(or L to go back)", uiTextSprites);
  horse->getMainSprite().set_scale(1);
}
