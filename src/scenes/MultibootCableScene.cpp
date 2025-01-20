#include "MultibootCableScene.h"

#include "../player/player.h"
#include "../utils/Math.h"
#include "../utils/gba-link-connection/LinkCableMultiboot.hpp"

#include "bn_keypad.h"

#define FILE_NAME "LinkUniversal_fullmb.gba"

using u32 = Link::u32;
using u16 = Link::u16;
using u8 = Link::u8;

MultibootCableScene::MultibootCableScene(const GBFS_FILE* _fs)
    : VideoScene(_fs) {}

void MultibootCableScene::init() {
  VideoScene::init();

  textGeneratorAccent.generate({0, 0}, "Press A to send!", uiTextSprites);
}

void MultibootCableScene::update() {
  VideoScene::update();

  if (bn::keypad::a_pressed())
    sendRom();
}

void MultibootCableScene::sendRom() {
  unsigned long romSize;
  const u8* romToSend = (const u8*)gbfs_get_obj(fs, FILE_NAME, &romSize);

  IS_SENDING = true;
  bool success = linkCableMultibootAsync->sendRom(romToSend, romSize);
  BN_LOG(success);
  while (linkCableMultibootAsync->getState() !=
         LinkCableMultiboot::Async::State::STOPPED) {
    // bn::core::update();
  }
  IS_SENDING = false;

  textGenerator.generate(
      {0, 10},
      "Result: " + bn::to_string<32>(linkCableMultibootAsync->getResult()),
      uiTextSprites);
}
