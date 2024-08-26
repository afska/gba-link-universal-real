// (0) Include the header
#include "utils/gba-link-connection/LinkUniversal.hpp"

#include "player/player.h"
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

static const GBFS_FILE* fs = find_first_gbfs_file(0);
bn::optional<bn::unique_ptr<Scene>> scene;

void ISR_VBlank();

void update() {
  bn::core::update();
  player_update(0, [](unsigned current) {});
}

int main() {
  bn::core::init(ISR_VBlank);  // << call LINK_UNIVERSAL_ISR_VBLANK()

  // (1) Create a LinkUniversal instance
  linkUniversal = new LinkUniversal(
      LinkUniversal::Protocol::CABLE, "LinkUNI",
      {.baudRate = LinkCable::BAUD_RATE_1,
       .timeout = LINK_CABLE_DEFAULT_TIMEOUT,
       .interval = Link::perFrame(3),
       .sendTimerId = 0},
      {.retransmission = true,
       .maxPlayers = 2,
       .timeout = LINK_WIRELESS_DEFAULT_TIMEOUT,
       .interval = Link::perFrame(3),
       .sendTimerId = 0,
       .asyncACKTimerId = LINK_WIRELESS_DEFAULT_ASYNC_ACK_TIMER_ID});

  // (2) Add the required interrupt service routines
  bn::memory::set_dma_enabled(true);  // < THIS IS NOT RECOMMENDED! JUST TESTING
  bn::hw::irq::set_isr(bn::hw::irq::id::SERIAL, LINK_UNIVERSAL_ISR_SERIAL);
  bn::hw::irq::set_isr(bn::hw::irq::id::TIMER0, LINK_UNIVERSAL_ISR_TIMER);
  bn::hw::irq::enable(bn::hw::irq::id::SERIAL);
  bn::hw::irq::enable(bn::hw::irq::id::TIMER0);

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
    update();
  }
}

BN_CODE_IWRAM void ISR_VBlank() {
  LINK_UNIVERSAL_ISR_VBLANK();
  player_onVBlank();
  bn::core::default_vblank_handler();
}
