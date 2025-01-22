// (0) Include the header
#include "utils/gba-link-connection/LinkUniversal.hpp"

#include "utils/gba-link-connection/LinkCableMultiboot.hpp"

#include "player/player.h"
#include "scenes/MultibootCableScene.h"
#include "scenes/StartScene.h"
#include "utils/gbfs/gbfs.h"

#include "../butano/hw/include/bn_hw_irq.h"
#include "bn_bg_palettes.h"
#include "bn_bgs_mosaic.h"
#include "bn_blending.h"
#include "bn_core.h"
#include "bn_memory.h"
#include "bn_optional.h"
#include "bn_sprite_palettes.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprites_mosaic.h"
#include "bn_unique_ptr.h"

LinkUniversal* linkUniversal = nullptr;
LinkCableMultiboot::Async* linkCableMultibootAsync = nullptr;

static const GBFS_FILE* fs = find_first_gbfs_file(0);
bn::optional<bn::unique_ptr<Scene>> scene;

void ISR_VBlank();
bn::unique_ptr<Scene> setNextScene(Screen nextScreen);
void transitionToNextScene();

int main() {
  bn::core::init(ISR_VBlank);  // << ISR_VBlank calls LINK_UNIVERSAL_ISR_VBLANK

  // (1) Create a LinkUniversal instance
  linkUniversal =
      new LinkUniversal(LinkUniversal::Protocol::AUTODETECT, "LinkUNI",
                        {.baudRate = LinkCable::BaudRate::BAUD_RATE_3,
                         .timeout = LINK_CABLE_DEFAULT_TIMEOUT,
                         .interval = Link::perFrame(4),
                         .sendTimerId = 0},
                        {.retransmission = true,
                         .maxPlayers = 2,
                         .timeout = LINK_WIRELESS_DEFAULT_TIMEOUT,
                         .interval = Link::perFrame(4),
                         .sendTimerId = 0});

  linkCableMultibootAsync = new LinkCableMultiboot::Async();

  // (2) Add the required interrupt service routines
  bn::memory::set_dma_enabled(false);
  // ^^^ DMA screws up interrupts and might cause packet loss!
  // ^^^ Most audio players also use DMA but it's not too terrible.
  bn::hw::irq::set_isr(bn::hw::irq::id::SERIAL,
                       LINK_CABLE_MULTIBOOT_ASYNC_ISR_SERIAL);
  // bn::hw::irq::set_isr(bn::hw::irq::id::TIMER0, LINK_UNIVERSAL_ISR_TIMER);
  bn::hw::irq::enable(bn::hw::irq::id::SERIAL);
  // bn::hw::irq::enable(bn::hw::irq::id::TIMER0);

  BN_ASSERT(fs != NULL,
            "GBFS file not found.\nUse the ROM that ends with .out.gba!");

  player_init();

  scene = bn::unique_ptr{(Scene*)new StartScene(fs)};
  scene->get()->init();

  // (3) Initialize the library
  linkUniversal->activate();

  while (true) {
    // (4) Sync
    linkUniversal->sync();

    scene->get()->update();

    if (scene->get()->hasNextScreen())
      transitionToNextScene();

    bn::core::update();
  }
}

BN_CODE_IWRAM void ISR_VBlank() {
  player_onVBlank();

  Link::_REG_IME = 1;
  player_update(0, [](unsigned current) {});
  Link::_REG_IME = 0;
  if (linkCableMultibootAsync != nullptr)
    LINK_CABLE_MULTIBOOT_ASYNC_ISR_VBLANK();
  bn::core::default_vblank_handler();
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
  linkUniversal->deactivate();

  switch (nextScreen) {
    case Screen::MAIN:
      return bn::unique_ptr{(Scene*)new StartScene(fs)};
    case Screen::MULTIBOOT_CABLE:
      return bn::unique_ptr{(Scene*)new MultibootCableScene(fs)};
    case Screen::MULTIBOOT_WIRELESS:
      return bn::unique_ptr{(Scene*)new StartScene(fs)};
    default: {
      BN_ERROR("Next screen not found?");
      return bn::unique_ptr{(Scene*)new StartScene(fs)};
    }
  }
}
