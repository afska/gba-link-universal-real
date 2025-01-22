#include "MultibootCableScene.h"

#include "../player/player.h"
#include "../utils/Math.h"
#include "../utils/gba-link-connection/LinkCableMultiboot.hpp"

#include "bn_core.h"  // TODO: REMOVE
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

  if (bn::keypad::a_pressed() && !isSending)
    sendRom();

  if (bn::keypad::b_pressed() && !isSending)
    sendRom(true);

  if (isSending && linkCableMultibootAsync->getState() ==
                       LinkCableMultiboot::Async::State::STOPPED) {
    isSending = false;
    textGenerator.generate(
        {0, 10},
        "Result: " + bn::to_string<32>(linkCableMultibootAsync->getResult()),
        uiTextSprites);
  }
}

void MultibootCableScene::sendRom(bool normal) {
  unsigned long romSize;
  const u8* romToSend = (const u8*)gbfs_get_obj(fs, FILE_NAME, &romSize);

  linkCableMultibootAsync->sendRom(
      romToSend, romSize,
      normal ? LinkCableMultiboot::TransferMode::SPI
             : LinkCableMultiboot::TransferMode::MULTI_PLAY);
  isSending = true;
}
