// Import the libraries
#include "utils/gba-link-connection/LinkCableMultiboot.hpp"
#include "utils/gba-link-connection/LinkUniversal.hpp"
#include "utils/gba-link-connection/LinkWirelessMultiboot.hpp"

#include "scenes/MultibootScene.h"
#include "scenes/StartScene.h"

#include "../butano/hw/include/bn_hw_irq.h"

LinkUniversal* linkUniversal = nullptr;
LinkCableMultiboot::Async* linkCableMultibootAsync = nullptr;
LinkWirelessMultiboot::Async* linkWirelessMultibootAsync = nullptr;

bn::optional<bn::unique_ptr<Scene>> scene;

void ISR_VBLANK();
void ISR_SERIAL();
void ISR_TIMER();
bn::unique_ptr<Scene> setNextScene(Screen nextScreen);
void transitionToNextScene();

int main() {
  // Initialize butano
  bn::core::init();

  // Create instances
  linkUniversal =
      new LinkUniversal(LinkUniversal::Protocol::AUTODETECT, "LinkUNI",
                        {.baudRate = LinkCable::BaudRate::BAUD_RATE_3,
                         .timeout = LINK_CABLE_DEFAULT_TIMEOUT,
                         .interval = Link::perFrame(4),
                         .sendTimerId = 1},
                        {.forwarding = true,
                         .retransmission = true,
                         .maxPlayers = 2,
                         .timeout = LINK_WIRELESS_DEFAULT_TIMEOUT,
                         .interval = Link::perFrame(4),
                         .sendTimerId = 1});
  linkCableMultibootAsync = new LinkCableMultiboot::Async();
  linkWirelessMultibootAsync = new LinkWirelessMultiboot::Async(
      "Multiboot", "Demo", 0x1234, 5, false, false, 30, 1);

  // Disable DMA
  bn::memory::set_dma_enabled(false);
  // ^^^ DMA stops interrupts and might cause packet loss!
  // ^^^ Most audio players also use DMA but it shouldn't affect LinkUniversal.

  // Add the required interrupt service routines
  bn::core::set_vblank_callback(ISR_VBLANK);
  bn::hw::irq::set_isr(bn::hw::irq::id::SERIAL, ISR_SERIAL);
  bn::hw::irq::enable(bn::hw::irq::id::SERIAL);
  bn::hw::irq::set_isr(bn::hw::irq::id::TIMER1, ISR_TIMER);
  bn::hw::irq::enable(bn::hw::irq::id::TIMER1);

  // Initialize scene
  scene = bn::unique_ptr{(Scene*)new StartScene()};
  scene->get()->init();

  // Main loop
  while (true) {
    scene->get()->update();

    if (scene->get()->hasNextScreen())
      transitionToNextScene();

    bn::core::update();
  }
}

BN_CODE_IWRAM void ISR_VBLANK() {
  LINK_UNIVERSAL_ISR_VBLANK();
  LINK_CABLE_MULTIBOOT_ASYNC_ISR_VBLANK();
  LINK_WIRELESS_MULTIBOOT_ASYNC_ISR_VBLANK();
}

BN_CODE_IWRAM void ISR_SERIAL() {
  LINK_UNIVERSAL_ISR_SERIAL();
  LINK_CABLE_MULTIBOOT_ASYNC_ISR_SERIAL();
  LINK_WIRELESS_MULTIBOOT_ASYNC_ISR_SERIAL();
}

BN_CODE_IWRAM void ISR_TIMER() {
  LINK_UNIVERSAL_ISR_TIMER();
  LINK_WIRELESS_MULTIBOOT_ASYNC_ISR_TIMER();
}

void transitionToNextScene() {
  auto nextScreen = scene->get()->getNextScreen();

  bn::bg_palettes::set_fade_intensity(0);
  bn::sprite_palettes::set_fade_intensity(0);
  bn::fixed alpha = 0;
  for (int i = 0; i < 10; i++) {
    alpha += 0.1;
    bn::bg_palettes::set_fade_intensity(alpha);
    bn::sprite_palettes::set_fade_intensity(alpha);

    bn::core::update();
  }

  scene->get()->destroy();
  scene.reset();
  bn::core::update();

  bn::bgs_mosaic::set_stretch(0);
  bn::sprites_mosaic::set_stretch(0);
  bn::blending::restore();

  scene = setNextScene(nextScreen);
  scene->get()->init();
  bn::core::update();

  for (int i = 0; i < 10; i++) {
    alpha -= 0.1;
    bn::bg_palettes::set_fade_intensity(alpha);
    bn::sprite_palettes::set_fade_intensity(alpha);

    scene->get()->update();
    bn::core::update();
  }
}

bn::unique_ptr<Scene> setNextScene(Screen nextScreen) {
  switch (nextScreen) {
    case Screen::MAIN:
      return bn::unique_ptr{(Scene*)new StartScene()};
    case Screen::MULTIBOOT_CABLE:
      return bn::unique_ptr{
          (Scene*)new MultibootScene(MultibootScene::Mode::CABLE)};
    case Screen::MULTIBOOT_WIRELESS:
      return bn::unique_ptr{
          (Scene*)new MultibootScene(MultibootScene::Mode::WIRELESS)};
    default: {
      BN_ERROR("Next screen not found?");
      return bn::unique_ptr{(Scene*)new StartScene()};
    }
  }
}
