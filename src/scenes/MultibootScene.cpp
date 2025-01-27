#include "MultibootScene.h"

#include "../utils/gba-link-connection/LinkCableMultiboot.hpp"
#include "../utils/gba-link-connection/LinkWirelessMultiboot.hpp"

#include "bn_keypad.h"

#define FILE_NAME "LinkUniversal_fullmb.gba"

using u32 = Link::u32;
using u16 = Link::u16;
using u8 = Link::u8;

MultibootScene::MultibootScene(Mode _mode, const GBFS_FILE* _fs)
    : VideoScene(_fs), mode(_mode) {}

void MultibootScene::init() {
  VideoScene::init();
  printInstructions();
  horse->customScale = true;
}

void MultibootScene::destroy() {
  reset();
}

void MultibootScene::update() {
  VideoScene::update();

  bool isSending = mode == Mode::CABLE
                       ? linkCableMultibootAsync->getState() !=
                             LinkCableMultiboot::Async::State::STOPPED
                       : linkWirelessMultibootAsync->getState() !=
                             LinkWirelessMultiboot::Async::State::STOPPED;
  bool hasResult = mode == Mode::CABLE
                       ? linkCableMultibootAsync->getResult(false) !=
                             LinkCableMultiboot::Async::Result::NONE
                       : linkWirelessMultibootAsync->getResult(false) !=
                             LinkWirelessMultiboot::Async::Result::NONE;

  // Print result, if any
  if (hasResult) {
    auto resultCode = mode == Mode::CABLE
                          ? (int)linkCableMultibootAsync->getResult(false)
                          : (int)linkWirelessMultibootAsync->getResult(false);

    uiTextSprites.clear();
    textGenerator.generate({0, 0}, "Result: " + bn::to_string<32>(resultCode),
                           uiTextSprites);
    if (bn::keypad::a_pressed()) {
      if (mode == Mode::CABLE)
        linkCableMultibootAsync->getResult();
      else
        linkWirelessMultibootAsync->getResult();
      printInstructions();
    }
    return;
  }

  if (isSending) {
    // Print send progress
    uiTextSprites.clear();

    auto percentage = getPercentage();

    horse->getMainSprite().set_scale(
        percentage > 0 ? 1 + bn::fixed(percentage) / 100 : 1);

    textGenerator.generate({0, -10},
                           "Sending... " + bn::to_string<32>(percentage) + "%",
                           uiTextSprites);
    textGenerator.generate({0, 10},
                           bn::to_string<32>(playerCount()) +
                               (mode == Mode::WIRELESS ? "/5" : "") +
                               " players",
                           uiTextSprites);

    if (bn::keypad::start_pressed())
      markReady();

    if (bn::keypad::b_pressed()) {
      reset();
      printInstructions();
    }
  } else {
    // Check inputs to start or exit
    if (mode == Mode::CABLE) {
      if (bn::keypad::a_pressed())
        sendRomViaCable();

      if (bn::keypad::b_pressed())
        sendRomViaCable(true);
    } else {
      if (bn::keypad::a_pressed())
        sendRomViaWirelessAdapter();
    }

    if (bn::keypad::l_pressed())
      setNextScreen(Screen::MAIN);

    if (bn::keypad::r_pressed())
      launch();
  }
}

void MultibootScene::sendRomViaCable(bool normalMode) {
  unsigned long romSize;
  const u8* romToSend = (const u8*)gbfs_get_obj(fs, FILE_NAME, &romSize);

  // Send ROM (cable)
  linkCableMultibootAsync->sendRom(
      romToSend, romSize, bn::keypad::start_held(),
      normalMode ? LinkCableMultiboot::TransferMode::SPI
                 : LinkCableMultiboot::TransferMode::MULTI_PLAY);
}

void MultibootScene::sendRomViaWirelessAdapter() {
  unsigned long romSize;
  const u8* romToSend = (const u8*)gbfs_get_obj(fs, FILE_NAME, &romSize);

  // Send ROM (wireless)
  linkWirelessMultibootAsync->sendRom(romToSend, romSize, "Multiboot", "Demo",
                                      0x7FFF, 5, bn::keypad::start_held(),
                                      false, 2);
}

void MultibootScene::printInstructions() {
  uiTextSprites.clear();
  if (mode == Mode::CABLE)
    textGeneratorAccent.generate({0, -20}, "A = Multi, B = SPI", uiTextSprites);
  else
    textGeneratorAccent.generate({0, -20}, "A = Send wirelessly",
                                 uiTextSprites);
  textGenerator.generate({0, 0}, "(L = back, R = launch)", uiTextSprites);
  textGenerator.generate({0, 20}, "(START = mark as ready)", uiTextSprites);
  horse->getMainSprite().set_scale(1);
}

unsigned MultibootScene::playerCount() {
  return mode == Mode::CABLE ? linkCableMultibootAsync->playerCount()
                             : linkWirelessMultibootAsync->playerCount();
}

unsigned MultibootScene::getPercentage() {
  return mode == Mode::CABLE ? linkCableMultibootAsync->getPercentage()
                             : linkWirelessMultibootAsync->getPercentage();
}

void MultibootScene::markReady() {
  if (mode == Mode::CABLE)
    linkCableMultibootAsync->markReady();
  else
    linkWirelessMultibootAsync->markReady();
}

void MultibootScene::reset() {
  if (mode == Mode::CABLE)
    linkCableMultibootAsync->reset();
  else
    linkWirelessMultibootAsync->reset();
}

void MultibootScene::launch() {
  unsigned long romSize;
  const u8* romToSend = (const u8*)gbfs_get_obj(fs, FILE_NAME, &romSize);

  Link::_REG_IME = 0;
  *((volatile u16*)0x04000082) = 0;  // (clear SOUNDCNT_H)

  void* EWRAM = (void*)0x02000000;
  for (u32 i = 0; i < romSize; i++)
    ((unsigned char*)EWRAM)[i] = romToSend[i];

  asm volatile(
      "mov r0, %0\n"
      "bx r0\n"
      :
      : "r"(EWRAM)
      : "r0");

  while (true)
    ;
}
